#pragma once

#include "fx/MasterEffectChain.h"
#include "state/PluginState.h"
#include "state/PresetManager.h"
#include "synth/engine/SynthLayer.h"
#include "utils/OutputSafetyProtector.h"
#include <JuceHeader.h>

//==============================================================================
/** Main plugin processor that owns state, synth layers, effects, and preset management. */
class EquinoxAudioProcessor : public AudioProcessor
{
public:
    /** Creates the processor and all long-lived runtime services. */
    EquinoxAudioProcessor();
    /** Destroys the processor and releases owned resources. */
    ~EquinoxAudioProcessor() override;

    /** Prepares DSP objects for playback at the supplied stream settings. */
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    /** Releases playback resources that are no longer needed. */
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    /** Returns whether the supplied bus layout is supported. */
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    /** Renders one audio block for all synth layers, effects, and safety processing. */
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    /** Creates the plugin editor instance. */
    AudioProcessorEditor* createEditor() override;
    /** Returns whether this processor exposes an editor. */
    bool hasEditor() const override;

    /** Returns the user-facing plugin name. */
    const String getName() const override;

    /** Returns whether the processor accepts MIDI input. */
    bool acceptsMidi() const override;
    /** Returns whether the processor produces MIDI output. */
    bool producesMidi() const override;
    /** Returns whether the processor acts as a pure MIDI effect. */
    bool isMidiEffect() const override;
    /** Returns the approximate plugin tail length in seconds. */
    double getTailLengthSeconds() const override;

    /** Returns the number of programs exposed through the legacy program API. */
    int getNumPrograms() override;
    /** Returns the currently selected legacy program index. */
    int getCurrentProgram() override;
    /** Selects a legacy program index. */
    void setCurrentProgram (int index) override;
    /** Returns the display name of a legacy program. */
    const String getProgramName (int index) override;
    /** Renames a legacy program. */
    void changeProgramName (int index, const String& newName) override;

    /** Serializes the current plugin state for the host. */
    void getStateInformation (MemoryBlock& destData) override;
    /** Restores plugin state provided by the host. */
    void setStateInformation (const void* data, int sizeInBytes) override;

    /** Returns the parameter state tree used by GUI bindings and DSP. */
    AudioProcessorValueTreeState& getParameters() noexcept;
    /** Returns the sample-state store used by the sampler engine. */
    AudioSampleValueTreeState& getAudioSampleState() noexcept;
    /** Returns the high-level plugin state service. */
    PluginState& getPluginState() noexcept;
    /** Returns the preset manager used by the editor and standalone workflow. */
    PresetManager& getPresetManager() noexcept;

    AudioProcessorValueTreeState parameterState;
    AudioSampleValueTreeState audioSampleState;
    PluginState pluginState;
    PresetManager presetManager;
    SynthLayer synthLayer1, synthLayer2, synthLayer3;
    MasterEffectChain masterEffectChain;
    OutputSafetyProtector outputSafetyProtector;

    MidiKeyboardState midiKeyboardState;

private:
    /** Builds the complete APVTS parameter layout for the plugin. */
    static AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout();

    double currentBPM = 120;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxAudioProcessor)
};
