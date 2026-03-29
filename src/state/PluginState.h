/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    PluginState.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "AudioSampleValueTreeState.h"
#include <JuceHeader.h>

/** Owns the plugin's serializable parameter, metadata, and sample-state services. */
class PluginState
{
public:
    /** Creates the state facade around the supplied parameter and sample state stores. */
    PluginState (AudioProcessorValueTreeState& parameterStateIn, AudioSampleValueTreeState& audioSampleStateIn);

    /** Returns mutable access to the parameter state tree. */
    AudioProcessorValueTreeState& parameters() noexcept;
    /** Returns read-only access to the parameter state tree. */
    const AudioProcessorValueTreeState& parameters() const noexcept;

    /** Returns mutable access to the audio-sample state store. */
    AudioSampleValueTreeState& audioSamples() noexcept;
    /** Returns read-only access to the audio-sample state store. */
    const AudioSampleValueTreeState& audioSamples() const noexcept;

    /** Returns a named child of the serialized plugin state tree. */
    ValueTree getChildWithName (const Identifier& type) const;

    /** Updates the current preset name metadata. */
    void setPresetName (const String& presetName);
    /** Returns the current preset name metadata value. */
    Value getPresetName();

    /** Serializes the current plugin state into a binary host-state block. */
    void saveStateToBinary (MemoryBlock& destinationData) const;
    /** Restores the plugin state from a binary host-state block. */
    void loadStateFromBinary (const void* data, int sizeInBytes);

    /** Saves the current plugin state to a preset file. */
    bool saveStateToFile (const File& file) const;
    /** Loads the plugin state from a preset file. */
    bool loadStateFromFile (const File& file);

    /** Restores the parameter and sample states to their captured defaults. */
    void resetStateToDefault();
    /** Returns a version counter that increments whenever external state is restored. */
    int getRestorationVersion() const noexcept;

private:
    /** Builds the current state as XML, optionally including preset metadata. */
    std::unique_ptr<XmlElement> getStateAsXml (bool includePresetName) const;
    /** Replaces the current state from XML, optionally overriding the preset name. */
    bool setStateFromXml (std::unique_ptr<XmlElement> stateXml, const String* presetNameOverride = nullptr);
    /** Counts children with the supplied tag name inside an XML element. */
    static int countChildrenWithTagName (const XmlElement& parent, StringRef tagName);

    ValueTree stateMetadata { "currentstate" };
    std::unique_ptr<XmlElement> initialStateXml;
    AudioProcessorValueTreeState& parameterState;
    AudioSampleValueTreeState& audioSampleState;
    std::atomic<int> restorationVersion { 0 };
};
