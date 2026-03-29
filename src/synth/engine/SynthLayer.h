/*
  ==============================================================================
       ______ ____   __  __ ____ _   __ ____  _  __
      / ____// __ \ / / / //  _// | / // __ \| |/ /
     / __/  / / / // / / / / / /  |/ // / / /|   /
    / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
   /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

   SynthLayer.h
   Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "state/ParameterModels.h"
#include "state/PluginState.h"
#include "synth/VoiceParameters.h"
#include "synth/engine/VoiceEngine.h"

/** Owns one user-facing synth layer and manages its state, parameters, and voice engines. */
class SynthLayer : private AudioProcessorValueTreeState::Listener
{
public:
    /** Identifies the currently selected engine type for the layer. */
    enum class SynthMode { oscillator, sampler };

    /** Creates a synth layer bound to the shared plugin state. */
    explicit SynthLayer (PluginState& pluginState, int instanceNumber);
    /** Removes listeners and releases owned engines. */
    ~SynthLayer() override;

    /** Performs one-time engine setup such as voice and sound creation. */
    void initialize();
    /** Returns the layer instance number used in parameter IDs. */
    int getInstanceNumber() const noexcept;
    /** Adds this layer's parameters to the global parameter layout. */
    void addParameters (std::vector<std::unique_ptr<RangedAudioParameter>>& params);
    /** Adds parameters for an arbitrary layer instance number. */
    static void addParametersForInstance (std::vector<std::unique_ptr<RangedAudioParameter>>& params, int instanceNumber);
    /** Prepares the layer engines for playback. */
    void prepareToPlay (const dsp::ProcessSpec& spec);
    /** Renders one audio block through the active layer engine. */
    void renderNextBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    /** Returns whether the layer needs processing work for the next block. */
    bool shouldProcessBlock (bool hasMidiMessages) const noexcept;

private:
    static constexpr int maxVoices = 16;

    /** Returns the currently selected voice engine. */
    [[nodiscard]] VoiceEngine& currentEngine() noexcept;
    /** Returns the currently selected voice engine. */
    [[nodiscard]] const VoiceEngine& currentEngine() const noexcept;
    /** Returns whether the active engine still has sounding voices. */
    [[nodiscard]] bool currentEngineHasActiveVoices() const noexcept;
    /** Returns whether structural state such as mode, mono, or sample data must be refreshed. */
    bool needsStructuralSync() const noexcept;
    /** Synchronizes mode, mono, activation, and sample-loading state. */
    void syncStructuralState();
    /** Returns the persistent sampler sound descriptor owned by the sample engine. */
    [[nodiscard]] SampleSynthSound* getSampleSound() const noexcept;
    /** Swaps a prepared sample payload into the persistent sampler sound. */
    void loadAudioSample (AudioSampleValueTreeState::AudioSamplePtr audioSample);
    /** Applies a newly prepared sample payload if one is available. */
    void loadSampleIfNeeded();
    /** Reapplies voice parameters if their cached values changed. */
    void applyVoiceParametersIfNeeded();
    /** Loads the common per-voice parameter set for this layer. */
    [[nodiscard]] VoiceParameters::Common loadCommonVoiceParameters() const;
    /** Loads the oscillator-specific voice parameter set for this layer. */
    [[nodiscard]] VoiceParameters::Oscillator loadOscillatorVoiceParameters() const;
    /** Loads the sampler-specific voice parameter set for this layer. */
    [[nodiscard]] VoiceParameters::Sampler loadSamplerVoiceParameters() const;
    /** Applies the cached parameter set to all oscillator voices. */
    void applyParametersToOscillatorVoices();
    /** Applies the cached parameter set to all sampler voices. */
    void applyParametersToSamplerVoices();
    /** Marks the layer dirty when one of its structural parameters changes. */
    void parameterChanged (const String& parameterID, float newValue) override;

    template <typename VoiceType>
    /** Applies the shared parameter set to one concrete voice type. */
    void applyCommonVoiceParameters (VoiceType& voice, const VoiceParameters::Common& parameters);

    PluginState& state;
    ParameterModels::SynthParameters parameterRefs;
    int instanceNum = 0;
    int currentSampleStateVersion = -1;
    int currentRestorationVersion = -1;
    double currentLoadedSampleRate = 0.0;
    double sampleRate = 0.0;
    SynthMode currentSynthMode = SynthMode::oscillator;
    SynthMode appliedVoiceParameterMode = SynthMode::oscillator;
    bool isSynthActive = false;
    bool isMonophonic = false;
    bool voiceParametersApplied = false;
    StringArray listenedParameterIds;
    std::atomic<bool> structuralStateDirty { true };
    std::atomic<bool> cachedIsActiveParameter { false };
    VoiceParameters::Common currentCommonParameters;
    VoiceParameters::Oscillator currentOscillatorParameters;
    VoiceParameters::Sampler currentSamplerParameters;
    VoiceEngine oscillatorEngine { VoiceEngine::VoiceType::oscillator };
    VoiceEngine sampleEngine { VoiceEngine::VoiceType::sampler };
};
