/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    OutputSafetyProtector.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Final-stage output protection that adds DC blocking, debug peak containment, and emergency muting. */
class OutputSafetyProtector
{
public:
    /** User-tunable parameters for the protection stage. */
    struct Parameters
    {
        /** Debug-build ceiling used for normal peak containment. Ignored in release builds. */
        float ceilingDb = -0.3f;
        float emergencyThresholdLinear = 4.0f;
        float dcBlockCutoffHz = 20.0f;
        float muteTimeMs = 250.0f;
        float recoveryTimeMs = 100.0f;
    };

    /** Prepares the protection stage for playback. */
    void prepare (const dsp::ProcessSpec& newSpec);
    /** Resets all internal state, counters, and DC blocker history. */
    void reset() noexcept;
    /** Replaces the active protection parameters. */
    void setParameters (const Parameters& newParameters) noexcept;
    /** Processes an output buffer through the safety stage. */
    void process (AudioBuffer<float>& buffer) noexcept;

    /** Returns whether protection is currently muting or recovering output. */
    [[nodiscard]] bool isProtectionActive() const noexcept;

private:
    /** Runtime state of the protection stage. */
    enum class State { normal, muted, recovering };

    /** Reason why protection was triggered. */
    enum class TriggerReason { invalidSample, runawayPeak };

    /** Captures the trigger reason and measured sample magnitude. */
    struct TriggerEvent
    {
        TriggerReason reason = TriggerReason::runawayPeak;
        float magnitude = 0.0f;
    };

    /** Applies the configured debug-build peak ceiling to a single sample. */
    [[nodiscard]] float applyCeiling (float sample) const noexcept;
    /** Processes one sample through the DC blocker for the supplied channel. */
    [[nodiscard]] float applyDcBlock (float sample, int channel) noexcept;
    /** Scans the current sample frame for invalid or runaway output. */
    [[nodiscard]] std::optional<TriggerEvent>
        getTriggerEvent (const float* const* channelData, int numChannels, int sampleIndex) const noexcept;
    /** Enters the muted state after a trigger condition has been detected. */
    void triggerProtection (const TriggerEvent& event) noexcept;
    /** Advances the mute/recovery state machine by one sample frame. */
    void advanceState() noexcept;

    Parameters parameters;
    dsp::ProcessSpec spec {};
    State state = State::normal;
    float ceilingLinear = 1.0f;
    float dcBlockPole = 0.0f;
    int muteSamplesRemaining = 0;
    int recoverySamplesRemaining = 0;
    float recoveryGain = 1.0f;
    float recoveryGainStep = 0.0f;
    int preparedNumChannels = 0;
    std::vector<float> previousInputs;
    std::vector<float> previousOutputs;
};
