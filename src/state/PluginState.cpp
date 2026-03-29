/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    PluginState.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "PluginState.h"

PluginState::PluginState (AudioProcessorValueTreeState& parameterStateIn, AudioSampleValueTreeState& audioSampleStateIn)
    : parameterState (parameterStateIn), audioSampleState (audioSampleStateIn)
{
    setPresetName ("Init");
    initialStateXml = getStateAsXml (true);
}

AudioProcessorValueTreeState& PluginState::parameters() noexcept { return parameterState; }

const AudioProcessorValueTreeState& PluginState::parameters() const noexcept { return parameterState; }

AudioSampleValueTreeState& PluginState::audioSamples() noexcept { return audioSampleState; }

const AudioSampleValueTreeState& PluginState::audioSamples() const noexcept { return audioSampleState; }

ValueTree PluginState::getChildWithName (const Identifier& type) const
{
    if (type == parameterState.state.getType())
        return parameterState.copyState();

    if (type == audioSampleState.state.getType())
        return audioSampleState.state.createCopy();

    return {};
}

void PluginState::setPresetName (const String& presetName)
{
    stateMetadata.setProperty ("presetname", presetName, nullptr);
}

Value PluginState::getPresetName() { return stateMetadata.getPropertyAsValue ("presetname", nullptr); }

std::unique_ptr<XmlElement> PluginState::getStateAsXml (bool includePresetName) const
{
    auto stateXml = std::make_unique<XmlElement> (stateMetadata.getType());

    if (includePresetName)
        stateXml->setAttribute ("presetname", stateMetadata.getProperty ("presetname").toString());

    if (auto parameterStateXml = parameterState.copyState().createXml())
        stateXml->addChildElement (parameterStateXml.release());

    if (auto audioSampleStateXml = audioSampleState.state.createCopy().createXml())
        stateXml->addChildElement (audioSampleStateXml.release());

    return stateXml;
}

int PluginState::countChildrenWithTagName (const XmlElement& parent, StringRef tagName)
{
    int childCount = 0;

    for (auto* childElement = parent.getFirstChildElement(); childElement != nullptr;
        childElement = childElement->getNextElement())
    {
        if (childElement->hasTagName (tagName))
            ++childCount;
    }

    return childCount;
}

bool PluginState::setStateFromXml (std::unique_ptr<XmlElement> stateXml, const String* presetNameOverride)
{
    if (stateXml == nullptr || !stateXml->hasTagName (stateMetadata.getType()))
        return false;

    if (presetNameOverride == nullptr && !stateXml->hasAttribute ("presetname"))
        return false;

    if (countChildrenWithTagName (*stateXml, parameterState.state.getType().toString()) != 1
        || countChildrenWithTagName (*stateXml, audioSampleState.state.getType().toString()) != 1)
    {
        return false;
    }

    auto* parameterStateXml = stateXml->getChildByName (parameterState.state.getType());
    auto* audioSampleStateXml = stateXml->getChildByName (audioSampleState.state.getType());

    if (parameterStateXml == nullptr || audioSampleStateXml == nullptr)
        return false;

    const auto parameterValueTree = ValueTree::fromXml (*parameterStateXml);
    const auto audioSampleValueTree = ValueTree::fromXml (*audioSampleStateXml);

    if (!parameterValueTree.isValid() || !audioSampleValueTree.isValid())
        return false;

    if (parameterValueTree.getType() != parameterState.state.getType()
        || audioSampleValueTree.getType() != audioSampleState.state.getType())
    {
        return false;
    }

    setPresetName (presetNameOverride != nullptr ? *presetNameOverride : stateXml->getStringAttribute ("presetname"));
    parameterState.replaceState (parameterValueTree);
    audioSampleState.replaceState (audioSampleValueTree);
    ++restorationVersion;
    return true;
}

void PluginState::saveStateToBinary (MemoryBlock& destinationData) const
{
    AudioPluginInstance::copyXmlToBinary (*getStateAsXml (true), destinationData);
}

void PluginState::loadStateFromBinary (const void* data, int sizeInBytes)
{
    setStateFromXml (AudioPluginInstance::getXmlFromBinary (data, sizeInBytes));
}

bool PluginState::saveStateToFile (const File& file) const { return getStateAsXml (false)->writeTo (file); }

bool PluginState::loadStateFromFile (const File& file)
{
    if (!file.existsAsFile())
        return false;

    XmlDocument xmlDocument (file);
    const auto presetNameFromFile = file.getFileNameWithoutExtension();
    return setStateFromXml (xmlDocument.getDocumentElement(), &presetNameFromFile);
}

void PluginState::resetStateToDefault()
{
    if (initialStateXml != nullptr)
        setStateFromXml (std::make_unique<XmlElement> (*initialStateXml));
}

int PluginState::getRestorationVersion() const noexcept { return restorationVersion.load(); }
