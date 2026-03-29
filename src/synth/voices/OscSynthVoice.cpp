/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    OscSynthVoice.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "OscSynthVoice.h"

OscSynthVoice::OscSynthVoice (bool isMonoVoice) : SynthVoice (isMonoVoice) {}

OscSynthVoice::~OscSynthVoice() {}

bool OscSynthVoice::canPlaySound (SynthesiserSound* sound)
{
    if (const auto* synthSound = dynamic_cast<const SynthSound*> (sound))
        return synthSound->getVoiceType() == SynthSound::VoiceType::oscillator;

    return false;
}

void OscSynthVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    auto* oscillatorSound = getOscillatorSound (sound);

    if (oscillatorSound == nullptr)
        return;

    ignoreUnused (oscillatorSound);

    if (!noteHasBeenTriggered)
    {
        oscillator1.setPhase (phase);
        oscillator2.setPhase (phase + (getRandomAnalogValue (true) / 100));
    }

    setBaseFrequencyFromMidiNote (midiNoteNumber);
    SynthVoice::startNote (midiNoteNumber, velocity, sound, currentPitchWheelPosition);
}

void OscSynthVoice::setPhase (float newPhase) { phase = newPhase; }

void OscSynthVoice::applyOscillatorParameters (const VoiceParameters::Oscillator& parameters)
{
    setWaveform (parameters.waveform);
    setPhase (parameters.phase);
}

float OscSynthVoice::getPhase() const { return phase; }

void OscSynthVoice::setWaveform (float selectedWaveform)
{
    oscillator1.setWaveform (static_cast<WavetableOscillator::Waveform> (roundToInt (selectedWaveform)));
    oscillator2.setWaveform (static_cast<WavetableOscillator::Waveform> (roundToInt (selectedWaveform)));
}

const OscSynthSound* OscSynthVoice::getOscillatorSound (const SynthesiserSound* sound) const noexcept
{
    const auto* synthSound = dynamic_cast<const SynthSound*> (sound);

    if (synthSound == nullptr || synthSound->getVoiceType() != SynthSound::VoiceType::oscillator)
        return nullptr;

    return static_cast<const OscSynthSound*> (sound);
}

double OscSynthVoice::getNextOscillatorSample (int channel, double mainFrequency, double detunedFrequency)
{
    if (channel == 0)
        return oscillator1.getNextSample (mainFrequency);

    return oscillator2.getNextSample (detunedFrequency);
}

void OscSynthVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (isVoiceActive())
    {
        jassert (numSamples <= voiceBuffer.getNumSamples());
        AudioBuffer<float> proxyBuffer (
            voiceBuffer.getArrayOfWritePointers(), voiceBuffer.getNumChannels(), 0, numSamples);
        proxyBuffer.clear();

        for (int sample = 0; sample < proxyBuffer.getNumSamples(); ++sample)
        {
            const auto mainFrequency = getCurrentFrequency();
            const auto detunedFrequency = getCurrentFrequency (getDetuneRatio());

            for (int channel = 0; channel < proxyBuffer.getNumChannels(); ++channel)
            {
                proxyBuffer.addSample (channel,
                    sample,
                    static_cast<float> (getNextOscillatorSample (channel, mainFrequency, detunedFrequency)));
            }

            advancePortamento();
        }
        addBufferToOutput (proxyBuffer, outputBuffer, startSample, numSamples);
    }
}
