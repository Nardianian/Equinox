/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    OutputSafetyProtector.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "OutputSafetyProtector.h"

namespace
{
    constexpr float minimumCeilingDb = -18.0f;
    constexpr float maximumCeilingDb = -0.01f;
    constexpr float minimumEmergencyThreshold = 1.0f;
    constexpr float minimumDcBlockCutoffHz = 5.0f;
    constexpr float maximumDcBlockCutoffHz = 40.0f;
    constexpr float minimumMuteTimeMs = 1.0f;
    constexpr float minimumRecoveryTimeMs = 1.0f;

#if JUCE_DEBUG
    constexpr bool normalCeilingEnabled = true;
#else
    constexpr bool normalCeilingEnabled = false;
#endif
}

void OutputSafetyProtector::prepare (const dsp::ProcessSpec& newSpec)
{
    spec = newSpec;
    preparedNumChannels = static_cast<int> (newSpec.numChannels);
    previousInputs.assign (static_cast<size_t> (preparedNumChannels), 0.0f);
    previousOutputs.assign (static_cast<size_t> (preparedNumChannels), 0.0f);
    setParameters (parameters);
    reset();
}

void OutputSafetyProtector::reset() noexcept
{
    state = State::normal;
    muteSamplesRemaining = 0;
    recoverySamplesRemaining = 0;
    recoveryGain = 1.0f;
    recoveryGainStep = 0.0f;
    std::ranges::fill (previousInputs, 0.0f);
    std::ranges::fill (previousOutputs, 0.0f);
}

void OutputSafetyProtector::setParameters (const Parameters& newParameters) noexcept
{
    parameters.ceilingDb = jlimit (minimumCeilingDb, maximumCeilingDb, newParameters.ceilingDb);
    parameters.emergencyThresholdLinear = jmax (minimumEmergencyThreshold, newParameters.emergencyThresholdLinear);
    parameters.dcBlockCutoffHz = jlimit (minimumDcBlockCutoffHz, maximumDcBlockCutoffHz, newParameters.dcBlockCutoffHz);
    parameters.muteTimeMs = jmax (minimumMuteTimeMs, newParameters.muteTimeMs);
    parameters.recoveryTimeMs = jmax (minimumRecoveryTimeMs, newParameters.recoveryTimeMs);
    ceilingLinear = Decibels::decibelsToGain (parameters.ceilingDb);
    dcBlockPole =
        spec.sampleRate > 0.0
            ? static_cast<float> (std::exp (
                  -MathConstants<double>::twoPi * static_cast<double> (parameters.dcBlockCutoffHz) / spec.sampleRate))
            : 0.0f;
}

bool OutputSafetyProtector::isProtectionActive() const noexcept { return state != State::normal; }

float OutputSafetyProtector::applyCeiling (float sample) const noexcept
{
    return jlimit (-ceilingLinear, ceilingLinear, sample);
}

float OutputSafetyProtector::applyDcBlock (float sample, int channel) noexcept
{
    jassert (isPositiveAndBelow (channel, preparedNumChannels));

    const auto filteredSample = sample - previousInputs[static_cast<size_t> (channel)]
                                + dcBlockPole * previousOutputs[static_cast<size_t> (channel)];

    previousInputs[static_cast<size_t> (channel)] = sample;
    previousOutputs[static_cast<size_t> (channel)] = filteredSample;

    return filteredSample;
}

std::optional<OutputSafetyProtector::TriggerEvent> OutputSafetyProtector::getTriggerEvent (
    const float* const* channelData,
    int numChannels,
    int sampleIndex) const noexcept
{
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto value = channelData[channel][sampleIndex];

        if (!std::isfinite (value))
            return TriggerEvent { .reason = TriggerReason::invalidSample,
                .magnitude = std::numeric_limits<float>::infinity() };

        const auto magnitude = std::abs (value);

        if (magnitude >= parameters.emergencyThresholdLinear)
            return TriggerEvent { .reason = TriggerReason::runawayPeak, .magnitude = magnitude };
    }

    return std::nullopt;
}

void OutputSafetyProtector::triggerProtection (const TriggerEvent& event) noexcept
{
#if JUCE_DEBUG
    const auto reasonText = event.reason == TriggerReason::invalidSample ? "invalid sample" : "runaway peak";
    Logger::writeToLog ("OutputSafetyProtector: protection triggered due to " + String (reasonText)
                        + " (magnitude=" + String (event.magnitude, 3) + ")");
#else
    ignoreUnused (event);
#endif

    state = State::muted;
    muteSamplesRemaining = jmax (1, roundToInt ((parameters.muteTimeMs * 0.001f) * static_cast<float> (spec.sampleRate)));
    recoverySamplesRemaining =
        jmax (1, roundToInt ((parameters.recoveryTimeMs * 0.001f) * static_cast<float> (spec.sampleRate)));
    recoveryGain = 0.0f;
    recoveryGainStep = 1.0f / static_cast<float> (recoverySamplesRemaining);
}

void OutputSafetyProtector::advanceState() noexcept
{
    if (state == State::muted)
    {
        if (--muteSamplesRemaining <= 0)
            state = State::recovering;

        return;
    }

    if (state == State::recovering)
    {
        recoveryGain = jmin (1.0f, recoveryGain + recoveryGainStep);

        if (--recoverySamplesRemaining <= 0 || recoveryGain >= 1.0f)
        {
            state = State::normal;
            recoveryGain = 1.0f;
            recoveryGainStep = 0.0f;
        }
    }
}

void OutputSafetyProtector::process (AudioBuffer<float>& buffer) noexcept
{
    if (spec.sampleRate <= 0.0 || buffer.getNumSamples() == 0)
        return;

    auto* const* channelData = buffer.getArrayOfWritePointers();
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int channel = 0; channel < numChannels; ++channel)
            channelData[channel][sample] = applyDcBlock (channelData[channel][sample], channel);

        if (const auto event = getTriggerEvent (channelData, numChannels, sample))
        {
            for (int channel = 0; channel < numChannels; ++channel)
                channelData[channel][sample] = 0.0f;

            triggerProtection (*event);
            advanceState();
            continue;
        }

        if (state == State::muted)
        {
            for (int channel = 0; channel < numChannels; ++channel)
                channelData[channel][sample] = 0.0f;

            advanceState();
            continue;
        }

        const auto gain = state == State::recovering ? recoveryGain : 1.0f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto processedSample = channelData[channel][sample] * gain;

            if constexpr (normalCeilingEnabled)
                processedSample = applyCeiling (processedSample);

            channelData[channel][sample] = processedSample;
        }

        advanceState();
    }
}
