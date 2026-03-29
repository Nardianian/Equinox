/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthVoice.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "SynthVoice.h"

namespace
{
    constexpr double millisecondsPerSecond = 1000.0;
}

SynthVoice::SynthVoice (bool isMonoVoice) : filterEnvelope (voiceFilter), monoMode (isMonoVoice) {}

SynthVoice::~SynthVoice() {}

// Used for preparing the voice and its objects
void SynthVoice::prepareVoice (const dsp::ProcessSpec& spec)
{
    setCurrentPlaybackSampleRate (spec.sampleRate);

    voiceFilter.prepareToPlay (spec);

    ampEnvelope.prepareToPlay (spec.sampleRate);

    filterEnvelope.prepareToPlay (spec.sampleRate);

    voiceBuffer.setSize (static_cast<int> (spec.numChannels), static_cast<int> (spec.maximumBlockSize));
}

void SynthVoice::setBaseFrequency (double frequency)
{
    previousFrequency = getCurrentFrequency();
    baseFrequency = frequency;
}

float SynthVoice::getKeyVelocity() const { return midiKeyVelocity; }

double SynthVoice::getDetuneRatio() const { return detuneRatio; }

double SynthVoice::getPortamentoRatio() const noexcept { return applyPortamento ? portamentoRatio : 1.0; }

double SynthVoice::getCurrentFrequency (double additionalRatio) const noexcept
{
    const auto staticPitchRatio = finePitchRatio * pitchTransposeRatio * analogPitchRatio * pitchBendRatio;
    return baseFrequency * staticPitchRatio * getPortamentoRatio() * additionalRatio;
}

void SynthVoice::advancePortamento() noexcept
{
    constexpr double targetValue = 1.0;

    if (!applyPortamento)
        return;

    portamentoRatio += portamentoDelta;

    const bool reachedTarget = (portamentoDelta >= 0.0 && portamentoRatio >= targetValue)
                               || (portamentoDelta <= 0.0 && portamentoRatio <= targetValue);

    if (reachedTarget)
    {
        applyPortamento = false;
        portamentoRatio = targetValue;
        portamentoDelta = 0.0;
    }
}

Filter& SynthVoice::getFilter() { return voiceFilter; }

FilterEnvelope& SynthVoice::getFilterEnvelope() { return filterEnvelope; }

AmpEnvelope& SynthVoice::getAmpEnvelope() { return ampEnvelope; }

void SynthVoice::setMonoMode (bool shouldBeMono) noexcept { monoMode = shouldBeMono; }

void SynthVoice::resetState()
{
    setKeyDown (false);
    setSustainPedalDown (false);
    setSostenutoPedalDown (false);
    clearCurrentNote();

    noteHasBeenTriggered = false;
    currentNoteNumber = -1;
    currentVoiceAmplitude = 0.0f;
    applyPortamento = false;
    baseFrequency = 0.0;
    previousFrequency = 0.0;
    midiKeyVelocity = 0.0f;
    detuneRatio = 1.0;
    finePitchRatio = 1.0;
    pitchTransposeRatio = 1.0;
    analogPitchRatio = 1.0;
    pitchBendAmount = 0.0f;
    pitchBendRatio = 1.0;
    portamentoRatio = 1.0;
    portamentoDelta = 0.0;

    ampEnvelope.noteOff();
    filterEnvelope.noteOff();
    voiceFilter.reset();
    voiceFilter.restoreCutoff();
    resetPlaybackState();
}

void SynthVoice::setPanning (float panValue) { panningValue = panValue; }

float SynthVoice::getPanning (int currentChannel) const
{
    float panning = 1.0f;

    // Turns down the left channel if panning right
    if (currentChannel == leftChannelIndex && panningValue > 0.0f)
    {
        return panning -= panningValue;
    }

    // Turns down the right channel if panning left
    if (currentChannel == rightChannelIndex && panningValue < 0.0f)
    {
        return panning += panningValue;
    }
    return panning; //returns panning at default value of 1 if panningValue is 0 (no active panning)
}

void SynthVoice::setPortamento (float timeMilliseconds)
{
    const auto currentSampleRate = getSampleRate();

    if (currentSampleRate <= 0.0)
    {
        portamentoSamples = 0;
        return;
    }

    const auto portamentoTimeSeconds = static_cast<double> (jmax (0.0f, timeMilliseconds)) / millisecondsPerSecond;
    portamentoSamples = jmax (0, roundToInt (portamentoTimeSeconds * currentSampleRate));
}

void SynthVoice::applyParameters (const VoiceParameters::Common& parameters)
{
    setAmplitude (parameters.amplitude);
    setDetune (parameters.detune);
    setFinePitch (parameters.finePitch);
    setAnalogFactor (parameters.analogValue);
    setPitchTranspose (parameters.pitchTranspose);
    setPanning (parameters.oscPanning);
    setPortamento (parameters.portamento);

    voiceFilter.setFilter (parameters.filter);
    voiceFilter.setMode (parameters.filter.mode);

    ampEnvelope.setEnvelope (parameters.ampEnvelope);
    filterEnvelope.setEnvelope (parameters.filterEnvelope);
    filterEnvelope.setCutoffLimit (parameters.filter.envelopeCutoffLimit);
}

bool SynthVoice::isMonoEnabled() const { return monoMode; }

void SynthVoice::setAmplitude (float ampValue) { masterAmplitude = jlimit (0.0f, 1.0f, ampValue); }

void SynthVoice::setDetune (float detuneValue) { detuneRatio = noteOffsetInHertz (detuneValue); }

void SynthVoice::setAnalogFactor (float analogFactorValue) { analogFactor = roundToInt (analogFactorValue); }

float SynthVoice::getAnalogPitch()
{
    if (analogFactor <= 1)
        return 1.0f;

    return static_cast<float> (noteOffsetInHertz (getRandomAnalogValue (false)));
}

float SynthVoice::getRandomAnalogValue (bool positiveValuesOnly)
{
    if (analogFactor <= 1)
        return 0.0f;

    if (positiveValuesOnly)
        return static_cast<float> (analogRandom.nextInt (analogFactor + 1));

    const auto minimumOffset = -((analogFactor / 2) + 1);
    return static_cast<float> (minimumOffset + analogRandom.nextInt (analogFactor + 1));
}

void SynthVoice::setPitchTranspose (float transposeValue)
{
    pitchTransposeRatio = noteOffsetInHertz (transposeValue * 100.0f);
}

void SynthVoice::setFinePitch (float pitchValue) { finePitchRatio = noteOffsetInHertz (pitchValue); }

void SynthVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    ignoreUnused (midiNoteNumber, sound);
    const bool isLegatoTransition = noteHasBeenTriggered;
    applyPortamento = false;
    portamentoRatio = 1.0;
    portamentoDelta = 0.0;
    setPitchBend (currentPitchWheelPosition);
    analogPitchRatio = getAnalogPitch();
    const auto currentFrequency = getCurrentFrequency();
    midiKeyVelocity = velocity;
    voiceFilter.restoreCutoff();
    const bool shouldApplyPortamento =
        isLegatoTransition && portamentoSamples > 0 && previousFrequency > 0.0 && currentFrequency > 0.0
        && !approximatelyEqual (static_cast<float> (previousFrequency), static_cast<float> (currentFrequency));

    if (shouldApplyPortamento)
    {
        applyPortamento = true;
        portamentoRatio = previousFrequency / currentFrequency;
        portamentoDelta = (1.0 - portamentoRatio) / static_cast<double> (portamentoSamples);
    }

    if (!noteHasBeenTriggered)
    {
        voiceFilter.reset();
        ampEnvelope.noteOn();
        filterEnvelope.noteOn();
        noteHasBeenTriggered = true;
    }
}

void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    ignoreUnused (velocity);
    if (monoMode && isKeyDown() && allowTailOff)
    {
        return;
    }

    if (!allowTailOff)
    {
        clearCurrentNote();
    }
    noteHasBeenTriggered = false;
    ampEnvelope.noteOff();
    filterEnvelope.noteOff();
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue) { setPitchBend (newPitchWheelValue); }

void SynthVoice::setPitchBend (int pitchWheelValue)
{
    if (pitchWheelValue > 8192)
    {
        pitchBendAmount = static_cast<float> (pitchWheelValue - 8192) / (16383 - 8192);
    }
    else
    {
        pitchBendAmount = static_cast<float> (8192 - pitchWheelValue) / -8192;
    }

    pitchBendRatio = noteOffsetInHertz (getPitchBendCents());
}

float SynthVoice::getPitchBendCents() const { return pitchBendAmount * pitchBendSemitones * 100; }

void SynthVoice::setBaseFrequencyFromMidiNote (int midiNote)
{
    setBaseFrequency (MidiMessage::getMidiNoteInHertz (midiNote));
    currentNoteNumber = midiNote;
}

double SynthVoice::noteInHertz (int midiNoteNumber, double centsOffset) const
{
    double hertz = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    hertz *= noteOffsetInHertz (centsOffset);
    return hertz;
}

double SynthVoice::noteOffsetInHertz (double offset) const { return std::pow (2.0, offset / 1200.0); }

void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    ignoreUnused (controllerNumber, newControllerValue);
}

void SynthVoice::addBufferToOutput (AudioBuffer<float>& bufferToAdd,
    AudioBuffer<float>& outputBuffer,
    int startSample,
    int numSamples)
{
    auto* const* sourceData = bufferToAdd.getArrayOfWritePointers();
    auto* const* outputData = outputBuffer.getArrayOfWritePointers();
    const auto numChannels = jmin (bufferToAdd.getNumChannels(), outputBuffer.getNumChannels());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        getFilterEnvelope().calculateNextValue();
        currentVoiceAmplitude = ampEnvelope.getNextSample() * getKeyVelocity();
        voiceFilter.processSampleFrame (sourceData, numChannels, sample);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            outputData[channel][startSample] +=
                sourceData[channel][sample] * currentVoiceAmplitude * masterAmplitude * getPanning (channel);
        }
        ++startSample;
    }

    if (!ampEnvelope.isActive())
    {
        stopNote (0.0f, false);
    }
}

void SynthVoice::resetPlaybackState() {}
