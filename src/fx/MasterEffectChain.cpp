/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    MasterEffectChain.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "MasterEffectChain.h"

namespace
{
    constexpr int parameterVersion = 1;
    constexpr float disabledMix = 0.0f;
    constexpr float normalizedMinimum = 0.0f;
    constexpr float normalizedMaximum = 1.0f;
    constexpr float normalizedStep = 0.01f;
    constexpr float chorusDepthMaximum = 0.15f;
    constexpr float chorusDepthStep = 0.0015f;
    constexpr float chorusDelayMinimum = 1.0f;
    constexpr float chorusDelayMaximum = 30.0f;
    constexpr float chorusDelayStep = 0.29f;
    constexpr float chorusFeedbackMinimum = -1.0f;
    constexpr float chorusFeedbackMaximum = 1.0f;
    constexpr float chorusFeedbackStep = 0.02f;
    constexpr float chorusRateMaximum = 5.0f;
    constexpr float phaserFrequencyMinimum = 100.0f;
    constexpr float phaserFrequencyMaximum = 22000.0f;
    constexpr float phaserFrequencyStep = 2.0f;
    constexpr float defaultPhaserFrequency = 11000.0f;
    constexpr float phaserFeedbackMinimum = -0.9f;
    constexpr float phaserFeedbackMaximum = 0.9f;
    constexpr float phaserFeedbackStep = 0.018f;
    constexpr float delayTimeMinimum = 1.0f;
    constexpr float delayTimeMaximum = 8.0f;
    constexpr float delayFeedbackMaximum = 100.0f;
    constexpr float unitSkew = 1.0f;

    using FloatRange = NormalisableRange<float>;

    std::unique_ptr<AudioParameterFloat>
        makeFloatParameter (const String& parameterId, const String& name, const FloatRange& range, float defaultValue)
    {
        return std::make_unique<AudioParameterFloat> (
            ParameterID (parameterId, parameterVersion), name, range, defaultValue);
    }

}

bool MasterEffectChain::Parameters::operator== (const Parameters& other) const noexcept
{
    return approximatelyEqual (chorusMix, other.chorusMix) && approximatelyEqual (chorusRate, other.chorusRate)
           && approximatelyEqual (chorusDepth, other.chorusDepth) && approximatelyEqual (chorusDelay, other.chorusDelay)
           && approximatelyEqual (chorusFeedback, other.chorusFeedback) && approximatelyEqual (phaserMix, other.phaserMix)
           && approximatelyEqual (phaserRate, other.phaserRate) && approximatelyEqual (phaserDepth, other.phaserDepth)
           && approximatelyEqual (phaserFrequency, other.phaserFrequency)
           && approximatelyEqual (phaserFeedback, other.phaserFeedback) && approximatelyEqual (reverbMix, other.reverbMix)
           && approximatelyEqual (reverbRoomsize, other.reverbRoomsize)
           && approximatelyEqual (reverbDamping, other.reverbDamping)
           && approximatelyEqual (reverbWidth, other.reverbWidth) && approximatelyEqual (delayMix, other.delayMix)
           && approximatelyEqual (delayTime, other.delayTime) && approximatelyEqual (delayFeedback, other.delayFeedback);
}

MasterEffectChain::MasterEffectChain (PluginState& pluginState)
    : parameters (pluginState.parameters()),
      state (pluginState),
      listenedParameterIds (ParameterIds::MasterFX::allParameterIds())
{
    for (const auto& parameterId : listenedParameterIds)
        state.parameters().addParameterListener (parameterId, this);
}

MasterEffectChain::~MasterEffectChain()
{
    for (const auto& parameterId : listenedParameterIds)
        state.parameters().removeParameterListener (parameterId, this);
}

void MasterEffectChain::initialize() { syncParameters(); }

void MasterEffectChain::addParameters (std::vector<std::unique_ptr<RangedAudioParameter>>& params)
{
    const auto normalizedRange = FloatRange (normalizedMinimum, normalizedMaximum, normalizedStep, unitSkew);
    const auto chorusRateRange = FloatRange (normalizedMinimum, chorusRateMaximum, normalizedStep, unitSkew);
    const auto chorusDepthRange = FloatRange (normalizedMinimum, chorusDepthMaximum, chorusDepthStep, unitSkew);
    const auto chorusDelayRange = FloatRange (chorusDelayMinimum, chorusDelayMaximum, chorusDelayStep, unitSkew);
    const auto chorusFeedbackRange =
        FloatRange (chorusFeedbackMinimum, chorusFeedbackMaximum, chorusFeedbackStep, unitSkew);
    const auto phaserFrequencyRange =
        FloatRange (phaserFrequencyMinimum, phaserFrequencyMaximum, phaserFrequencyStep, unitSkew);
    const auto phaserFeedbackRange =
        FloatRange (phaserFeedbackMinimum, phaserFeedbackMaximum, phaserFeedbackStep, unitSkew);
    const auto delayTimeRange = FloatRange (delayTimeMinimum, delayTimeMaximum, delayTimeMinimum, unitSkew);
    const auto delayFeedbackRange = FloatRange (normalizedMinimum, delayFeedbackMaximum, delayTimeMinimum, unitSkew);

    params.push_back (makeFloatParameter (ParameterIds::MasterFX::chorusMix, "Chorus mix", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::chorusRate, "Chorus rate", chorusRateRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::chorusDepth, "Chorus depth", chorusDepthRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::chorusDelay, "Chorus delay", chorusDelayRange, chorusDelayMinimum));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::chorusFeedback, "Chorus feedback", chorusFeedbackRange, disabledMix));

    params.push_back (makeFloatParameter (ParameterIds::MasterFX::phaserMix, "Phaser mix", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::phaserRate, "Phaser rate", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::phaserDepth, "Phaser depth", normalizedRange, disabledMix));
    params.push_back (makeFloatParameter (
        ParameterIds::MasterFX::phaserFrequency, "Phaser frequency", phaserFrequencyRange, defaultPhaserFrequency));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::phaserFeedback, "Phaser feedback", phaserFeedbackRange, disabledMix));

    params.push_back (makeFloatParameter (ParameterIds::MasterFX::reverbMix, "Reverb mix", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::reverbRoomsize, "Reverb roomsize", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::reverbDamping, "Reverb damping", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::reverbWidth, "Reverb width", normalizedRange, disabledMix));

    params.push_back (makeFloatParameter (ParameterIds::MasterFX::delayMix, "Delay mix", normalizedRange, disabledMix));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::delayTime, "Delay time", delayTimeRange, delayTimeMinimum));
    params.push_back (
        makeFloatParameter (ParameterIds::MasterFX::delayFeedback, "Delay feedback", delayFeedbackRange, disabledMix));
}

void MasterEffectChain::prepareToPlay (const dsp::ProcessSpec& spec)
{
    reset();

    chorusFX.prepare (spec);
    phaserFX.prepare (spec);
    reverbFX.prepare (spec);
    delayFX.prepare (spec);
    syncParameters();
}

void MasterEffectChain::reset()
{
    chorusFX.reset();
    phaserFX.reset();
    reverbFX.reset();
    delayFX.reset();
}

void MasterEffectChain::process (AudioBuffer<float>& bufferToProcess, double bpm)
{
    if (needsSync())
        syncParameters();

    if (!hasActiveEffects.load (std::memory_order_acquire))
        return;

    chorusFX.process (bufferToProcess);
    phaserFX.process (bufferToProcess);
    reverbFX.process (bufferToProcess);
    delayFX.process (bufferToProcess, bpm);
}

bool MasterEffectChain::shouldProcessBlock() const noexcept
{
    return needsSync() || hasActiveEffects.load (std::memory_order_acquire);
}

bool MasterEffectChain::needsSync() const noexcept
{
    return parametersDirty.load (std::memory_order_acquire) || currentRestorationVersion != state.getRestorationVersion();
}

MasterEffectChain::Parameters MasterEffectChain::loadParameters() const
{
    return { .chorusMix = parameters.chorusMix.load (std::memory_order_relaxed),
        .chorusRate = parameters.chorusRate.load (std::memory_order_relaxed),
        .chorusDepth = parameters.chorusDepth.load (std::memory_order_relaxed),
        .chorusDelay = parameters.chorusDelay.load (std::memory_order_relaxed),
        .chorusFeedback = parameters.chorusFeedback.load (std::memory_order_relaxed),
        .phaserMix = parameters.phaserMix.load (std::memory_order_relaxed),
        .phaserRate = parameters.phaserRate.load (std::memory_order_relaxed),
        .phaserDepth = parameters.phaserDepth.load (std::memory_order_relaxed),
        .phaserFrequency = parameters.phaserFrequency.load (std::memory_order_relaxed),
        .phaserFeedback = parameters.phaserFeedback.load (std::memory_order_relaxed),
        .reverbMix = parameters.reverbMix.load (std::memory_order_relaxed),
        .reverbRoomsize = parameters.reverbRoomsize.load (std::memory_order_relaxed),
        .reverbDamping = parameters.reverbDamping.load (std::memory_order_relaxed),
        .reverbWidth = parameters.reverbWidth.load (std::memory_order_relaxed),
        .delayMix = parameters.delayMix.load (std::memory_order_relaxed),
        .delayTime = parameters.delayTime.load (std::memory_order_relaxed),
        .delayFeedback = parameters.delayFeedback.load (std::memory_order_relaxed) };
}

void MasterEffectChain::syncParameters()
{
    const auto restorationVersion = state.getRestorationVersion();
    const bool restorationChanged = currentRestorationVersion != restorationVersion;
    const bool parametersWereDirty = parametersDirty.exchange (false, std::memory_order_acq_rel);
    const bool shouldRefreshParameters = restorationChanged || parametersWereDirty;

    if (restorationChanged)
        reset();

    if (!shouldRefreshParameters)
        return;

    const auto nextParameters = loadParameters();
    const bool effectsAreActive = nextParameters.chorusMix > disabledMix || nextParameters.phaserMix > disabledMix
                                  || nextParameters.reverbMix > disabledMix || nextParameters.delayMix > disabledMix;

    if (!restorationChanged && nextParameters == currentParameters)
        return;

    applyParameters (nextParameters);
    hasActiveEffects.store (effectsAreActive, std::memory_order_release);
    currentParameters = nextParameters;
    currentRestorationVersion = restorationVersion;
}

void MasterEffectChain::applyParameters (const Parameters& parametersToApply)
{
    chorusFX.setParameters (parametersToApply.chorusRate,
        parametersToApply.chorusDepth,
        parametersToApply.chorusDelay,
        parametersToApply.chorusFeedback,
        parametersToApply.chorusMix);

    phaserFX.setParameters (parametersToApply.phaserRate,
        parametersToApply.phaserDepth,
        parametersToApply.phaserFrequency,
        parametersToApply.phaserFeedback,
        parametersToApply.phaserMix);

    reverbFX.setParameters (parametersToApply.reverbRoomsize,
        parametersToApply.reverbDamping,
        parametersToApply.reverbWidth,
        parametersToApply.reverbMix);

    delayFX.setParameters (parametersToApply.delayTime, parametersToApply.delayFeedback, parametersToApply.delayMix);
}

void MasterEffectChain::parameterChanged (const String& parameterID, float newValue)
{
    ignoreUnused (parameterID, newValue);
    parametersDirty.store (true, std::memory_order_release);
}
