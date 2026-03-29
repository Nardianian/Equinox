/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    MasterEffectChain.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "ChorusFX.h"
#include "DelayFX.h"
#include "PhaserFX.h"
#include "ReverbFX.h"
#include "state/ParameterModels.h"
#include "state/PluginState.h"

/** Coordinates the master chorus, phaser, reverb, and delay processors. */
class MasterEffectChain : private AudioProcessorValueTreeState::Listener
{
public:
    /** Creates the effect chain backed by the shared plugin state. */
    explicit MasterEffectChain (PluginState& state);
    /** Removes listeners and releases owned DSP objects. */
    ~MasterEffectChain() override;

    /** Initializes effect-specific one-time state such as parameter listeners. */
    void initialize();
    /** Adds all master-effect parameters to the global parameter layout. */
    static void addParameters (std::vector<std::unique_ptr<RangedAudioParameter>>& params);
    /** Prepares the effect processors for playback. */
    void prepareToPlay (const dsp::ProcessSpec& spec);
    /** Resets all effect state and internal buffers. */
    void reset();
    /** Processes the supplied audio buffer through the active master effects. */
    void process (AudioBuffer<float>& bufferToProcess, double bpm);
    /** Returns whether any effect work is pending for the next block. */
    bool shouldProcessBlock() const noexcept;

private:
    /** Cached view of all master-effect parameters needed during rendering. */
    struct Parameters
    {
        float chorusMix = 0.0f;
        float chorusRate = 0.0f;
        float chorusDepth = 0.0f;
        float chorusDelay = 0.0f;
        float chorusFeedback = 0.0f;
        float phaserMix = 0.0f;
        float phaserRate = 0.0f;
        float phaserDepth = 0.0f;
        float phaserFrequency = 0.0f;
        float phaserFeedback = 0.0f;
        float reverbMix = 0.0f;
        float reverbRoomsize = 0.0f;
        float reverbDamping = 0.0f;
        float reverbWidth = 0.0f;
        float delayMix = 0.0f;
        float delayTime = 0.0f;
        float delayFeedback = 0.0f;

        bool operator== (const Parameters& other) const noexcept;
    };

    /** Returns whether effect parameters or restored state need to be re-applied. */
    bool needsSync() const noexcept;
    /** Reads the latest effect parameter values from the APVTS atomics. */
    [[nodiscard]] Parameters loadParameters() const;
    /** Synchronizes cached effect state with the latest parameter values. */
    void syncParameters();
    /** Applies a fully loaded effect parameter set to the underlying processors. */
    void applyParameters (const Parameters& parameters);
    /** Marks the chain dirty when a listened parameter changes. */
    void parameterChanged (const String& parameterID, float newValue) override;

    ChorusFX chorusFX;
    PhaserFX phaserFX;
    ReverbFX reverbFX;
    DelayFX delayFX;
    ParameterModels::MasterEffectParameters parameters;
    Parameters currentParameters;
    PluginState& state;
    StringArray listenedParameterIds;
    std::atomic<bool> parametersDirty { true };
    std::atomic<bool> hasActiveEffects { false };
    int currentRestorationVersion = -1;
};
