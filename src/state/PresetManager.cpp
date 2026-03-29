/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    PresetManager.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "PresetManager.h"
#include <JuceHeader.h>
#include <PresetBinaryData.h>

namespace
{
    void assignMismatchReason (String* mismatchReason, String message)
    {
        if (mismatchReason != nullptr)
            *mismatchReason = std::move (message);
    }

    float getCurrentParameterValue (AudioProcessorValueTreeState& parameters, const String& parameterId)
    {
        if (auto* rawValue = parameters.getRawParameterValue (parameterId))
            return rawValue->load();

        jassertfalse;
        return 0.0f;
    }

    float getDefaultParameterValue (AudioProcessorValueTreeState& parameters, const String& parameterId)
    {
        if (const auto* parameter = parameters.getParameter (parameterId))
            return parameters.getParameterRange (parameterId).convertFrom0to1 (parameter->getDefaultValue());

        jassertfalse;
        return 0.0f;
    }

    float canonicalizeParameterValue (AudioProcessorValueTreeState& parameters, const String& parameterId, float value)
    {
        return parameters.getParameterRange (parameterId).snapToLegalValue (value);
    }

    bool parameterValuesMatch (AudioProcessorValueTreeState& parameters,
        const String& parameterId,
        float currentValue,
        float presetValue)
    {
        const auto parameterRange = parameters.getParameterRange (parameterId);
        const auto parameterStep = parameterRange.interval;
        const auto tolerance =
            parameterStep > 0.0f ? (parameterStep * 0.5f) + 1.0e-6f : jmax (1.0e-5f, std::abs (presetValue) * 1.0e-6f);

        return std::abs (currentValue - presetValue) <= tolerance;
    }

}

PresetManager::PresetManager (PluginState& stateIn, int synthLayerCount, const File& initialDirectory)
    : currentPresetName (Value (stateIn.getPresetName())),
      currentPresetHasBeenEdited (var (false)),
      state (stateIn),
      listenedParameterIds (ParameterIds::allParameterIds (synthLayerCount))
{
    for (const auto& parameterId : listenedParameterIds)
        state.parameters().addParameterListener (parameterId, this);

    const auto presetDirectory = initialDirectory != File() ? initialDirectory : getDefaultPresetDirectory();
    loadDirectory (presetDirectory.getFullPathName(), true);
    startTimerHz (15);
}

PresetManager::~PresetManager()
{
    stopTimer();

    for (const auto& parameterId : listenedParameterIds)
        state.parameters().removeParameterListener (parameterId, this);
}

File PresetManager::getDefaultPresetDirectory()
{
    return File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("Equinox").getChildFile ("Presets");
}

String PresetManager::sanitizePresetName (String presetName)
{
    presetName = presetName.trim();

    if (presetName.isEmpty())
        presetName = "Init";

    presetName = File::createLegalFileName (presetName);

    if (presetName.isEmpty())
        presetName = "Init";

    return presetName.substring (0, maxPresetNameLength);
}

void PresetManager::loadDirectory (const String& newDirectoryPath, bool createDirectoryIfNotExisting)
{
    directoryPath = newDirectoryPath;
    directory = File (directoryPath);
    const bool directoryAlreadyExists = directory.exists();

    if (createDirectoryIfNotExisting && !directory.exists())
        directory.createDirectory();

    const bool shouldSeedFactoryPresets = !directoryAlreadyExists && directory.exists();
    seedFactoryPresetsIfNeeded (shouldSeedFactoryPresets);

    refreshPresetFiles();

    if (currentPresetExistsAsFile())
    {
        reconcileEditedStateAgainstCurrentPreset();
        return;
    }

    if (directoryContainsFiles())
        loadPresetFromFile (presetFiles.getFirst());

    reconcileEditedStateAgainstCurrentPreset();
}

void PresetManager::savePreset()
{
    FileChooser fileChooser (
        "Save preset", getPresetFileFromName (currentPresetName.toString()), "*.equinox", true, false, nullptr);

    if (!fileChooser.browseForFileToSave (true))
        return;

    auto presetFile = fileChooser.getResult();

    savePresetToFile (presetFile);
}

void PresetManager::loadPreset()
{
    FileChooser fileChooser ("Load preset", directory, "*.equinox", true, false, nullptr);

    if (fileChooser.browseForFileToOpen())
        loadPresetFromFile (fileChooser.getResult());
}

bool PresetManager::savePresetToFile (const File& presetFile)
{
    const auto sanitizedPresetName = sanitizePresetName (presetFile.getFileNameWithoutExtension());
    const auto sanitizedPresetFile = presetFile.getParentDirectory().getChildFile (sanitizedPresetName + ".equinox");

    if (!sanitizedPresetFile.getParentDirectory().exists() && !sanitizedPresetFile.getParentDirectory().createDirectory())
        return false;

    beginPresetTransaction();
    state.setPresetName (sanitizedPresetName);

    const auto didSave = state.saveStateToFile (sanitizedPresetFile);

    if (didSave)
    {
        refreshPresetFiles();
        currentPresetIndex = getPresetIndexFromName (currentPresetName.toString());
    }

    endPresetTransaction (didSave);
    return didSave;
}

bool PresetManager::loadPresetFromFile (const File& presetFile)
{
    beginPresetTransaction();
    const auto didLoad = state.loadStateFromFile (presetFile);

    if (didLoad)
    {
        refreshPresetFiles();
        currentPresetIndex = getPresetIndexFromName (currentPresetName.toString());
    }

    endPresetTransaction (didLoad);
    return didLoad;
}

void PresetManager::getNextPreset()
{
    if (!directoryContainsFiles())
        return;

    const auto nextIndex = currentPresetIndex < (presetFiles.size() - 1) ? currentPresetIndex + 1 : 0;
    loadPresetFromFile (presetFiles.getReference (nextIndex));
}

void PresetManager::getPreviousPreset()
{
    if (!directoryContainsFiles())
        return;

    const auto previousIndex = currentPresetIndex > 0 ? currentPresetIndex - 1 : (presetFiles.size() - 1);
    loadPresetFromFile (presetFiles.getReference (previousIndex));
}

void PresetManager::overwritePreset()
{
    if (!currentPresetExistsAsFile())
        return;

    savePresetToFile (presetFiles.getReference (currentPresetIndex));
}

void PresetManager::initializePreset()
{
    beginPresetTransaction();
    state.resetStateToDefault();
    currentPresetIndex = getPresetIndexFromName (currentPresetName.toString());
    endPresetTransaction (true);
}

void PresetManager::beginExternalStateRestore() { suppressEditedFlag = true; }

void PresetManager::syncAfterExternalStateRestore (bool didRestoreState)
{
    suppressEditedFlag = false;
    editedStateNeedsReconcile.store (false);

    if (!didRestoreState)
        return;

    refreshPresetFiles();
    reconcileEditedStateAgainstCurrentPreset();
}

Value* PresetManager::getCurrentPresetName() { return &currentPresetName; }

bool PresetManager::currentPresetExistsAsFile() const
{
    return isPositiveAndBelow (currentPresetIndex, presetFiles.size())
           && presetFiles.getReference (currentPresetIndex).existsAsFile();
}

Value* PresetManager::getCurrentPresetHasBeenEdited() { return &currentPresetHasBeenEdited; }

int PresetManager::getPresetIndexFromName (const String& presetName) const
{
    const auto presetFile = getPresetFileFromName (presetName);

    for (int index = 0; index < presetFiles.size(); ++index)
        if (presetFiles.getReference (index) == presetFile)
            return index;

    return -1;
}

File PresetManager::getPresetFileFromName (const String& presetName) const
{
    return directory.getChildFile (presetName + ".equinox");
}

bool PresetManager::currentStateMatchesPresetFile (const File& presetFile, String* mismatchReason) const
{
    if (!presetFile.existsAsFile())
    {
        assignMismatchReason (mismatchReason, "Preset file does not exist");
        return false;
    }

    XmlDocument xmlDocument (presetFile);
    const auto presetXml = xmlDocument.getDocumentElement();

    if (presetXml == nullptr)
    {
        assignMismatchReason (mismatchReason, "Preset XML could not be parsed");
        return false;
    }

    auto* parameterStateXml = presetXml->getChildByName (state.parameters().state.getType());
    auto* audioSampleStateXml = presetXml->getChildByName (state.audioSamples().state.getType());

    if (parameterStateXml == nullptr || audioSampleStateXml == nullptr)
    {
        assignMismatchReason (mismatchReason, "Preset XML is missing parameterstate or audiosamplestate");
        return false;
    }

    const auto presetParameterState = ValueTree::fromXml (*parameterStateXml);
    const auto presetAudioSampleState = ValueTree::fromXml (*audioSampleStateXml);

    if (!presetParameterState.isValid() || !presetAudioSampleState.isValid())
    {
        assignMismatchReason (mismatchReason, "Preset ValueTrees could not be reconstructed from XML");
        return false;
    }

    for (const auto& parameterId : listenedParameterIds)
    {
        const auto presetParameter = presetParameterState.getChildWithProperty ("id", parameterId);
        const auto currentValue = getCurrentParameterValue (state.parameters(), parameterId);
        const auto presetValue = canonicalizeParameterValue (state.parameters(),
            parameterId,
            presetParameter.isValid() && presetParameter.hasProperty ("value")
                ? static_cast<float> (presetParameter.getProperty ("value"))
                : getDefaultParameterValue (state.parameters(), parameterId));

        if (!parameterValuesMatch (state.parameters(), parameterId, currentValue, presetValue))
        {
            assignMismatchReason (mismatchReason,
                "Parameter mismatch: " + parameterId + " current=" + String (currentValue, 9) + " preset="
                    + String (presetValue, 9));
            return false;
        }
    }

    if (!presetAudioSampleState.isEquivalentTo (state.audioSamples().state.createCopy()))
    {
        assignMismatchReason (mismatchReason, "Audio sample state differs from preset");
        return false;
    }

    return true;
}

void PresetManager::seedFactoryPresetsIfNeeded (bool shouldSeedFactoryPresets) const
{
    if (!shouldSeedFactoryPresets || directory != getDefaultPresetDirectory() || !directory.exists())
        return;

    for (int resourceIndex = 0; resourceIndex < PresetResources::namedResourceListSize; ++resourceIndex)
    {
        const auto* resourceName = PresetResources::namedResourceList[resourceIndex];
        const auto* originalFilename = PresetResources::originalFilenames[resourceIndex];

        if (resourceName == nullptr || originalFilename == nullptr)
            continue;

        const auto originalResourceName = String::fromUTF8 (originalFilename);
        const auto lastSeparatorIndex =
            jmax (originalResourceName.lastIndexOfChar ('/'), originalResourceName.lastIndexOfChar ('\\'));
        const auto presetFilename =
            lastSeparatorIndex >= 0 ? originalResourceName.substring (lastSeparatorIndex + 1) : originalResourceName;

        if (!presetFilename.endsWithIgnoreCase (".equinox"))
            continue;

        const auto presetFile = directory.getChildFile (presetFilename);

        if (presetFile.existsAsFile())
            continue;

        int presetSize = 0;
        const auto* presetData = PresetResources::getNamedResource (resourceName, presetSize);

        if (presetData == nullptr || presetSize <= 0)
            continue;

        presetFile.replaceWithData (presetData, static_cast<size_t> (presetSize));
    }
}

bool PresetManager::directoryContainsFiles() const { return !presetFiles.isEmpty(); }

void PresetManager::refreshPresetFiles()
{
    presetFiles = directory.findChildFiles (File::findFiles, false, "*.equinox");
    std::sort (presetFiles.begin(), presetFiles.end(), [] (const File& lhs, const File& rhs) {
        return lhs.getFileNameWithoutExtension().compareNatural (rhs.getFileNameWithoutExtension()) < 0;
    });

    currentPresetIndex = getPresetIndexFromName (currentPresetName.toString());
}

void PresetManager::reconcileEditedStateAgainstCurrentPreset()
{
    if (currentPresetExistsAsFile())
    {
        currentPresetHasBeenEdited = !currentStateMatchesPresetFile (presetFiles.getReference (currentPresetIndex));
        return;
    }

    currentPresetHasBeenEdited = directoryContainsFiles();
}

void PresetManager::beginPresetTransaction() { suppressEditedFlag = true; }

void PresetManager::endPresetTransaction (bool shouldClearEditedState)
{
    suppressEditedFlag = false;
    editedStateNeedsReconcile.store (false);

    if (shouldClearEditedState)
        currentPresetHasBeenEdited = false;
}

void PresetManager::parameterChanged (const String& parameterID, float newValue)
{
    ignoreUnused (parameterID, newValue);

    if (!suppressEditedFlag)
        editedStateNeedsReconcile.store (true);
}

void PresetManager::timerCallback()
{
    if (editedStateNeedsReconcile.exchange (false))
        reconcileEditedStateAgainstCurrentPreset();
}
