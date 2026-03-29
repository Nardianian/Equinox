/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    SynthLayer.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "SynthLayer.h"

namespace
{
    constexpr int parameterVersion = 1;
    constexpr float boolThreshold = 0.5f;
    constexpr float normalizedMinimum = 0.0f;
    constexpr float normalizedMaximum = 1.0f;
    constexpr float envelopeTimeMaximumSeconds = 10.0f;
    constexpr float envelopeTimeStep = 0.002f;
    constexpr float envelopeTimeSkew = 0.4f;
    constexpr float percentageStep = 0.001f;
    constexpr float centeredPercentageMinimum = -1.0f;
    constexpr float centeredPercentageStep = 0.01f;
    constexpr float semitoneDetuneRange = 20.0f;
    constexpr float centDetuneRange = 100.0f;
    constexpr float pitchTransposeRange = 24.0f;
    constexpr float analogMinimum = 1.0f;
    constexpr float analogMaximum = 40.0f;
    constexpr float filterCutoffMinimum = 5.0f;
    constexpr float filterCutoffMaximum = 22050.0f;
    constexpr float envelopeCutoffMinimum = 200.0f;
    constexpr float envelopeCutoffStep = 0.1f;
    constexpr float resonanceMaximum = 0.9f;
    constexpr float resonanceStep = 0.009f;
    constexpr float driveMaximum = 15.0f;
    constexpr float driveStep = 0.14f;
    constexpr float portamentoMaximumMilliseconds = 500.0f;
    constexpr float portamentoStepMilliseconds = 0.1f;
    constexpr float portamentoSkew = 0.35f;
    constexpr float sampleStartMaximum = 0.9f;
    constexpr float sampleStartSkew = 0.5f;
    constexpr float defaultAmpDecay = 1.0f;
    constexpr float defaultAmpSustain = 0.8f;
    constexpr float defaultEnvelopeCutoffLimit = 10000.0f;
    constexpr float defaultAmplitude = 0.7f;
    constexpr float defaultCutoff = filterCutoffMaximum;
    constexpr bool firstLayerStartsActive = true;

    using FloatRange = NormalisableRange<float>;

    bool loadParameterAsBool (const std::atomic<float>& parameter) noexcept
    {
        return parameter.load (std::memory_order_relaxed) >= boolThreshold;
    }

    SynthLayer::SynthMode loadSynthMode (const std::atomic<float>& parameter) noexcept
    {
        return static_cast<SynthLayer::SynthMode> (roundToInt (parameter.load (std::memory_order_relaxed)));
    }

    std::unique_ptr<AudioParameterFloat>
        makeFloatParameter (const String& parameterId, const String& name, const FloatRange& range, float defaultValue)
    {
        return std::make_unique<AudioParameterFloat> (
            ParameterID (parameterId, parameterVersion), name, range, defaultValue);
    }
}

SynthLayer::SynthLayer (PluginState& pluginState, int instanceNumber)
    : state (pluginState),
      parameterRefs (pluginState.parameters(), instanceNumber),
      instanceNum (instanceNumber),
      cachedIsActiveParameter (loadParameterAsBool (parameterRefs.isActive))
{
    listenedParameterIds.add (parameterRefs.ids.isActive);
    listenedParameterIds.add (parameterRefs.ids.synthMode);
    listenedParameterIds.add (parameterRefs.ids.monoEnabled);

    for (const auto& parameterId : listenedParameterIds)
        state.parameters().addParameterListener (parameterId, this);
}

SynthLayer::~SynthLayer()
{
    for (const auto& parameterId : listenedParameterIds)
        state.parameters().removeParameterListener (parameterId, this);
}

void SynthLayer::initialize()
{
    oscillatorEngine.addVoices (maxVoices);
    sampleEngine.addVoices (maxVoices);
    sampleEngine.addSound (new SampleSynthSound());
}

int SynthLayer::getInstanceNumber() const noexcept { return instanceNum; }

void SynthLayer::prepareToPlay (const dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    oscillatorEngine.prepare (spec);
    sampleEngine.prepare (spec);
    syncStructuralState();
    applyVoiceParametersIfNeeded();
}

void SynthLayer::addParameters (std::vector<std::unique_ptr<RangedAudioParameter>>& params)
{
    addParametersForInstance (params, instanceNum);
}

void SynthLayer::addParametersForInstance (std::vector<std::unique_ptr<RangedAudioParameter>>& params, int instanceNumber)
{
    const auto ids = ParameterIds::Synth (instanceNumber);
    const auto envelopeRange =
        FloatRange (normalizedMinimum, envelopeTimeMaximumSeconds, envelopeTimeStep, envelopeTimeSkew);
    const auto sustainRange = FloatRange (normalizedMinimum, normalizedMaximum, envelopeTimeStep, normalizedMaximum);
    const auto envelopeCutoffRange =
        FloatRange (envelopeCutoffMinimum, filterCutoffMaximum, envelopeCutoffStep, envelopeTimeSkew);
    const auto normalizedRange = FloatRange (normalizedMinimum, normalizedMaximum, percentageStep, normalizedMaximum);
    const auto detuneRange = FloatRange (-semitoneDetuneRange, semitoneDetuneRange, analogMinimum, normalizedMaximum);
    const auto finePitchRange = FloatRange (-centDetuneRange, centDetuneRange, analogMinimum, normalizedMaximum);
    const auto panningRange =
        FloatRange (centeredPercentageMinimum, normalizedMaximum, centeredPercentageStep, normalizedMaximum);
    const auto transposeRange = FloatRange (-pitchTransposeRange, pitchTransposeRange, analogMinimum, normalizedMaximum);
    const auto analogRange = FloatRange (analogMinimum, analogMaximum, analogMinimum, normalizedMaximum);
    const auto cutoffRange = FloatRange (filterCutoffMinimum, filterCutoffMaximum, analogMinimum, 0.3f);
    const auto resonanceRange = FloatRange (normalizedMinimum, resonanceMaximum, resonanceStep, normalizedMaximum);
    const auto driveRange = FloatRange (analogMinimum, driveMaximum, driveStep, normalizedMaximum);
    const auto portamentoRange =
        FloatRange (normalizedMinimum, portamentoMaximumMilliseconds, portamentoStepMilliseconds, portamentoSkew);
    const auto sampleStartRange = FloatRange (normalizedMinimum, sampleStartMaximum, percentageStep, sampleStartSkew);

    params.push_back (makeFloatParameter (ids.ampAttack, "AmpAttack", envelopeRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.ampDecay, "AmpDecay", envelopeRange, defaultAmpDecay));
    params.push_back (makeFloatParameter (ids.ampSustain, "AmpSustain", sustainRange, defaultAmpSustain));
    params.push_back (makeFloatParameter (ids.ampRelease, "AmpRelease", envelopeRange, normalizedMinimum));

    params.push_back (makeFloatParameter (ids.filterAttack, "FilterAttack", envelopeRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.filterDecay, "FilterDecay", envelopeRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.filterSustain, "FilterSustain", sustainRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.filterRelease, "FilterRelease", envelopeRange, normalizedMinimum));
    params.push_back (makeFloatParameter (
        ids.envelopeCutoffLimit, "EnvelopeCutoffLimit", envelopeCutoffRange, defaultEnvelopeCutoffLimit));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID (ids.waveform, parameterVersion), "Waveform", normalizedMinimum, 4.0f, normalizedMinimum));

    const bool initializeAsActive = instanceNumber == 1 ? firstLayerStartsActive : false;
    params.push_back (std::make_unique<AudioParameterBool> (
        ParameterID (ids.isActive, parameterVersion), "On/Off", initializeAsActive));

    params.push_back (makeFloatParameter (ids.amplitude, "Amplitude", normalizedRange, defaultAmplitude));
    params.push_back (makeFloatParameter (ids.detune, "Detune", detuneRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.finePitch, "FinePitch", finePitchRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.oscPanning, "OscPanning", panningRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.pitchTranspose, "PitchTranspose", transposeRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.analogValue, "AnalogValue", analogRange, analogMinimum));

    params.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID (ids.filterType, parameterVersion), "FilterType", normalizedMinimum, 3.0f, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.cutoff, "Cutoff", cutoffRange, defaultCutoff));
    params.push_back (makeFloatParameter (ids.resonance, "Resonance", resonanceRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.drive, "Drive", driveRange, analogMinimum));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID (ids.synthMode, parameterVersion),
        "SynthMode",
        normalizedMinimum,
        normalizedMaximum,
        normalizedMinimum));
    params.push_back (
        std::make_unique<AudioParameterBool> (ParameterID (ids.monoEnabled, parameterVersion), "Mono", false));
    params.push_back (makeFloatParameter (ids.portamento, "Portamento", portamentoRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.phase, "Phase", normalizedRange, normalizedMinimum));
    params.push_back (makeFloatParameter (ids.sampleStartTime, "Sample start time", sampleStartRange, normalizedMinimum));
}

void SynthLayer::renderNextBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    if (needsStructuralSync())
        syncStructuralState();

    if (!isSynthActive)
        return;

    applyVoiceParametersIfNeeded();

    currentEngine().renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

bool SynthLayer::shouldProcessBlock (bool hasMidiMessages) const noexcept
{
    const auto& audioSamples = state.audioSamples();
    const auto preparedSampleStateVersion = audioSamples.getPreparedStateVersion();
    const auto requestedSampleStateVersion = audioSamples.getStateVersion();

    return structuralStateDirty.load (std::memory_order_acquire)
           || currentRestorationVersion != state.getRestorationVersion()
           || currentSampleStateVersion != preparedSampleStateVersion
           || preparedSampleStateVersion != requestedSampleStateVersion || currentEngineHasActiveVoices()
           || (cachedIsActiveParameter.load (std::memory_order_acquire) && hasMidiMessages);
}

VoiceEngine& SynthLayer::currentEngine() noexcept
{
    return currentSynthMode == SynthMode::oscillator ? oscillatorEngine : sampleEngine;
}

const VoiceEngine& SynthLayer::currentEngine() const noexcept
{
    return currentSynthMode == SynthMode::oscillator ? oscillatorEngine : sampleEngine;
}

bool SynthLayer::currentEngineHasActiveVoices() const noexcept { return currentEngine().hasActiveVoices(); }

bool SynthLayer::needsStructuralSync() const noexcept
{
    const auto& audioSamples = state.audioSamples();
    const auto preparedSampleStateVersion = audioSamples.getPreparedStateVersion();
    const auto requestedSampleStateVersion = audioSamples.getStateVersion();

    return structuralStateDirty.load (std::memory_order_acquire)
           || currentRestorationVersion != state.getRestorationVersion()
           || currentSampleStateVersion != preparedSampleStateVersion
           || preparedSampleStateVersion != requestedSampleStateVersion
           || !approximatelyEqual (currentLoadedSampleRate, sampleRate);
}

void SynthLayer::syncStructuralState()
{
    const auto& audioSamples = state.audioSamples();
    const auto sampleStateVersion = audioSamples.getStateVersion();
    const auto preparedSampleStateVersion = audioSamples.getPreparedStateVersion();
    const auto restorationVersion = state.getRestorationVersion();
    const bool parametersWereDirty = structuralStateDirty.exchange (false, std::memory_order_acq_rel);
    const bool restorationChanged = currentRestorationVersion != restorationVersion;
    const bool shouldRefreshStructure = parametersWereDirty || restorationChanged;
    const auto nextIsActive = shouldRefreshStructure ? loadParameterAsBool (parameterRefs.isActive) : isSynthActive;
    const auto nextMonoEnabled = shouldRefreshStructure ? loadParameterAsBool (parameterRefs.monoEnabled) : isMonophonic;
    const auto nextSynthMode = shouldRefreshStructure ? loadSynthMode (parameterRefs.synthMode) : currentSynthMode;
    const bool activeStateChanged = shouldRefreshStructure && nextIsActive != isSynthActive;
    const bool monoModeChanged = shouldRefreshStructure && nextMonoEnabled != isMonophonic;
    const bool synthModeChanged = shouldRefreshStructure && nextSynthMode != currentSynthMode;
    const bool sampleStateChanged = currentSampleStateVersion != preparedSampleStateVersion;
    const bool samplePreparationPending = preparedSampleStateVersion != sampleStateVersion;
    const bool sampleRateChanged = !approximatelyEqual (currentLoadedSampleRate, sampleRate);

    if (shouldRefreshStructure)
        cachedIsActiveParameter.store (nextIsActive, std::memory_order_release);

    if (synthModeChanged || monoModeChanged || activeStateChanged || restorationChanged)
    {
        oscillatorEngine.clearAllCurrentNotes();
        sampleEngine.clearAllCurrentNotes();
        currentSampleStateVersion = -1;
        voiceParametersApplied = false;
    }

    if (shouldRefreshStructure)
    {
        isSynthActive = nextIsActive;
        isMonophonic = nextMonoEnabled;
        currentSynthMode = nextSynthMode;
        oscillatorEngine.setMonoEnabled (isMonophonic);
        sampleEngine.setMonoEnabled (isMonophonic);
    }

    if (sampleStateChanged || samplePreparationPending || synthModeChanged || restorationChanged || sampleRateChanged)
    {
        loadSampleIfNeeded();
    }

    currentRestorationVersion = restorationVersion;
}

void SynthLayer::parameterChanged (const String& parameterID, float newValue)
{
    if (parameterID == parameterRefs.ids.isActive)
        cachedIsActiveParameter.store (newValue >= boolThreshold, std::memory_order_release);

    structuralStateDirty.store (true, std::memory_order_release);
}

SampleSynthSound* SynthLayer::getSampleSound() const noexcept
{
    if (sampleEngine.getNumSounds() <= 0)
        return nullptr;

    return dynamic_cast<SampleSynthSound*> (sampleEngine.getSound (0).get());
}

void SynthLayer::loadAudioSample (AudioSampleValueTreeState::AudioSamplePtr audioSample)
{
    auto* sampleSound = getSampleSound();

    if (sampleSound == nullptr)
        return;

    sampleSound->setSample (std::move (audioSample));
}

void SynthLayer::loadSampleIfNeeded()
{
    currentLoadedSampleRate = sampleRate;

    if (currentSynthMode != SynthMode::sampler)
    {
        loadAudioSample (nullptr);
        currentSampleStateVersion = state.audioSamples().getPreparedStateVersion();
        return;
    }

    const auto requestedSampleStateVersion = state.audioSamples().getStateVersion();
    const auto preparedSampleStateVersion = state.audioSamples().getPreparedStateVersion();

    if (preparedSampleStateVersion != requestedSampleStateVersion)
    {
        loadAudioSample (nullptr);
        return;
    }

    loadAudioSample (state.audioSamples().getPreparedAudioSample (parameterRefs.ids.sampleStateId));
    currentSampleStateVersion = preparedSampleStateVersion;
}

VoiceParameters::Common SynthLayer::loadCommonVoiceParameters() const
{
    return { .amplitude = parameterRefs.amplitude.load (std::memory_order_relaxed),
        .detune = parameterRefs.detune.load (std::memory_order_relaxed),
        .finePitch = parameterRefs.finePitch.load (std::memory_order_relaxed),
        .oscPanning = parameterRefs.oscPanning.load (std::memory_order_relaxed),
        .pitchTranspose = parameterRefs.pitchTranspose.load (std::memory_order_relaxed),
        .analogValue = parameterRefs.analogValue.load (std::memory_order_relaxed),
        .portamento = parameterRefs.portamento.load (std::memory_order_relaxed),
        .ampEnvelope = { .attack = parameterRefs.ampAttack.load (std::memory_order_relaxed),
            .decay = parameterRefs.ampDecay.load (std::memory_order_relaxed),
            .sustain = parameterRefs.ampSustain.load (std::memory_order_relaxed),
            .release = parameterRefs.ampRelease.load (std::memory_order_relaxed) },
        .filterEnvelope = { .attack = parameterRefs.filterAttack.load (std::memory_order_relaxed),
            .decay = parameterRefs.filterDecay.load (std::memory_order_relaxed),
            .sustain = parameterRefs.filterSustain.load (std::memory_order_relaxed),
            .release = parameterRefs.filterRelease.load (std::memory_order_relaxed) },
        .filter = { .mode = parameterRefs.filterType.load (std::memory_order_relaxed),
            .cutoff = parameterRefs.cutoff.load (std::memory_order_relaxed),
            .resonance = parameterRefs.resonance.load (std::memory_order_relaxed),
            .drive = parameterRefs.drive.load (std::memory_order_relaxed),
            .envelopeCutoffLimit = parameterRefs.envelopeCutoffLimit.load (std::memory_order_relaxed) } };
}

VoiceParameters::Oscillator SynthLayer::loadOscillatorVoiceParameters() const
{
    return { .waveform = parameterRefs.waveform.load (std::memory_order_relaxed),
        .phase = parameterRefs.phase.load (std::memory_order_relaxed) };
}

VoiceParameters::Sampler SynthLayer::loadSamplerVoiceParameters() const
{
    return { .sampleStartTime = parameterRefs.sampleStartTime.load (std::memory_order_relaxed) };
}

void SynthLayer::applyVoiceParametersIfNeeded()
{
    const auto nextCommonParameters = loadCommonVoiceParameters();
    const auto nextOscillatorParameters = loadOscillatorVoiceParameters();
    const auto nextSamplerParameters = loadSamplerVoiceParameters();

    const bool parametersChanged = !voiceParametersApplied || currentCommonParameters != nextCommonParameters
                                   || currentOscillatorParameters != nextOscillatorParameters
                                   || currentSamplerParameters != nextSamplerParameters
                                   || appliedVoiceParameterMode != currentSynthMode;

    if (!parametersChanged)
        return;

    currentCommonParameters = nextCommonParameters;
    currentOscillatorParameters = nextOscillatorParameters;
    currentSamplerParameters = nextSamplerParameters;
    appliedVoiceParameterMode = currentSynthMode;
    voiceParametersApplied = true;

    if (currentSynthMode == SynthMode::oscillator)
        applyParametersToOscillatorVoices();
    else
        applyParametersToSamplerVoices();
}

template <typename VoiceType>
void SynthLayer::applyCommonVoiceParameters (VoiceType& voice, const VoiceParameters::Common& parameters)
{
    voice.applyParameters (parameters);
}

void SynthLayer::applyParametersToOscillatorVoices()
{
    const auto voiceCount = oscillatorEngine.getNumVoices();

    for (int voiceIndex = 0; voiceIndex < voiceCount; ++voiceIndex)
    {
        auto* voice = dynamic_cast<OscSynthVoice*> (oscillatorEngine.getVoice (voiceIndex));

        if (voice == nullptr)
            continue;

        applyCommonVoiceParameters (*voice, currentCommonParameters);
        voice->applyOscillatorParameters (currentOscillatorParameters);
    }
}

void SynthLayer::applyParametersToSamplerVoices()
{
    const auto voiceCount = sampleEngine.getNumVoices();

    for (int voiceIndex = 0; voiceIndex < voiceCount; ++voiceIndex)
    {
        auto* voice = dynamic_cast<SampleSynthVoice*> (sampleEngine.getVoice (voiceIndex));

        if (voice == nullptr)
            continue;

        applyCommonVoiceParameters (*voice, currentCommonParameters);
        voice->applySamplerParameters (currentSamplerParameters);
    }
}
