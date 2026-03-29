/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    PresetManager.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "state/ParameterIds.h"
#include "state/PluginState.h"
#include <JuceHeader.h>

/** Handles preset file browsing, save/load workflow, and edited-state tracking. */
class PresetManager : private AudioProcessorValueTreeState::Listener, private Timer
{
public:
    static constexpr int maxPresetNameLength = 20;

    /** Creates the preset manager and optionally loads an initial preset directory. */
    PresetManager (PluginState& state, int synthLayerCount, const File& initialDirectory = {});
    /** Removes parameter listeners and releases cached preset metadata. */
    ~PresetManager() override;

    /** Returns the default user preset directory. */
    static File getDefaultPresetDirectory();
    /** Sanitizes a preset name for filesystem use and enforces the display length limit. */
    static String sanitizePresetName (String presetName);

    /** Loads the preset directory and optionally creates it if missing. */
    void loadDirectory (const String& directoryPath, bool createDirectoryIfNotExisting = false);
    /** Opens the interactive save dialog. */
    void savePreset();
    /** Opens the interactive load dialog. */
    void loadPreset();
    /** Saves the current state to the supplied preset file. */
    bool savePresetToFile (const File& presetFile);
    /** Loads the supplied preset file into the plugin state. */
    bool loadPresetFromFile (const File& presetFile);
    /** Loads the next preset in the current directory. */
    void getNextPreset();
    /** Loads the previous preset in the current directory. */
    void getPreviousPreset();
    /** Overwrites the currently selected preset file with the current state. */
    void overwritePreset();
    /** Resets the plugin state to the default initialized preset. */
    void initializePreset();
    /** Suspends edited-state tracking while external state restoration is in progress. */
    void beginExternalStateRestore();
    /** Reconciles preset bookkeeping after host-driven state restoration. */
    void syncAfterExternalStateRestore (bool didRestoreState);

    /** Returns a listenable value containing the current preset name. */
    Value* getCurrentPresetName();
    /** Returns whether the current preset still exists as a file on disk. */
    bool currentPresetExistsAsFile() const;
    /** Returns a listenable value indicating whether the preset has unsaved edits. */
    Value* getCurrentPresetHasBeenEdited();

private:
    /** Seeds bundled factory presets when the default preset folder is created. */
    void seedFactoryPresetsIfNeeded (bool shouldSeedFactoryPresets) const;
    /** Returns whether the current in-memory state matches the supplied preset file exactly. */
    bool currentStateMatchesPresetFile (const File& presetFile, String* mismatchReason = nullptr) const;
    /** Returns the directory index of a preset by name, or -1 if missing. */
    int getPresetIndexFromName (const String& presetName) const;
    /** Returns the preset file path that corresponds to a preset name. */
    File getPresetFileFromName (const String& presetName) const;
    /** Returns whether the current preset directory contains any preset files. */
    bool directoryContainsFiles() const;
    /** Refreshes the cached preset file list and current index. */
    void refreshPresetFiles();
    /** Updates the edited flag by comparing the current state against the current preset file. */
    void reconcileEditedStateAgainstCurrentPreset();
    /** Suppresses edited-state tracking while a preset transaction is in progress. */
    void beginPresetTransaction();
    /** Finalizes a preset transaction and optionally clears the edited flag. */
    void endPresetTransaction (bool shouldClearEditedState);
    /** Marks the current preset as edited after normal parameter changes. */
    void parameterChanged (const String& parameterID, float newValue) override;
    /** Reconciles deferred edited-state requests on the message thread. */
    void timerCallback() override;

    Array<File> presetFiles;
    File directory;
    String directoryPath;
    Value currentPresetName;
    Value currentPresetHasBeenEdited;
    int currentPresetIndex = -1;
    PluginState& state;
    StringArray listenedParameterIds;
    bool suppressEditedFlag = false;
    std::atomic<bool> editedStateNeedsReconcile { false };
};
