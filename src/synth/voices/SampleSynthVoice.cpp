/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    SampleSynthVoice.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "SampleSynthVoice.h"

namespace
{
    constexpr double semitonesPerOctave = 12.0;
    constexpr int interpolationLookaheadSamples = 2;
} // namespace

SampleSynthVoice::SampleSynthVoice (bool isMonoVoice) : SynthVoice (isMonoVoice) {}

SampleSynthVoice::~SampleSynthVoice() {}

bool SampleSynthVoice::canPlaySound (SynthesiserSound* sound)
{
    if (const auto* synthSound = dynamic_cast<const SynthSound*> (sound))
        return synthSound->getVoiceType() == SynthSound::VoiceType::sampler;

    return false;
}

void SampleSynthVoice::startNote (int midiNoteNumber,
    float velocity,
    SynthesiserSound* sound,
    int currentPitchWheelPosition)
{
    const auto* sampleSound = getSamplerSound (sound);

    if (sampleSound == nullptr)
        return;

    if (auto sample = sampleSound->getSample())
    {
        const auto& sampleData = sample->sampleSource;
        currentNoteNumber = midiNoteNumber;
        const auto semitoneDelta = static_cast<double> (midiNoteNumber - sampleSound->getMidiRootNote());
        const auto pitchRatio = std::pow (2.0, semitoneDelta / semitonesPerOctave);
        setBaseFrequency ((pitchRatio * static_cast<double> (sample->samplerate)) / getSampleRate());

        sampleLength = sampleData.getNumSamples();

        if (!noteHasBeenTriggered)
        {
            sourceSamplePositionLeft = getSampleStartTime();
            sourceSamplePositionRight = getSampleStartTime();
        }

        SynthVoice::startNote (midiNoteNumber, velocity, sound, currentPitchWheelPosition);
    }
}

void SampleSynthVoice::applySamplerParameters (const VoiceParameters::Sampler& parameters)
{
    setSampleStartTime (parameters.sampleStartTime);
}

void SampleSynthVoice::setSampleStartTime (float startTime)
{
    if (startTime > 1.0f || startTime < 0.0f)
        return;

    sampleStart = startTime;
}

float SampleSynthVoice::getSampleStartTime() const { return static_cast<float> (sampleStart * sampleLength); }

float SampleSynthVoice::getNextSamplerSample (int channel, const float* const inL, const float* const inR)
{
    if (channel == 0)
        return readSampleAtPosition (sourceSamplePositionLeft, inL);

    return readSampleAtPosition (sourceSamplePositionRight, inR);
}

void SampleSynthVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (isVoiceActive() && sourceSamplePositionLeft < sampleLength && sourceSamplePositionRight < sampleLength)
    {
        auto* playingSound = static_cast<SampleSynthSound*> (getCurrentlyPlayingSound().get());

        if (playingSound == nullptr)
            return;

        auto loadedSample = playingSound->getSample();

        if (loadedSample == nullptr)
            return;

        const auto& sampleData = loadedSample->sampleSource;
        jassert (numSamples <= voiceBuffer.getNumSamples());
        AudioBuffer<float> proxyBuffer (
            voiceBuffer.getArrayOfWritePointers(), voiceBuffer.getNumChannels(), 0, numSamples);
        proxyBuffer.clear();

        const float* const inL = sampleData.getReadPointer (0);
        const float* const inR = sampleData.getNumChannels() > 1 ? sampleData.getReadPointer (1) : nullptr;

        for (int sample = 0; sample < proxyBuffer.getNumSamples(); ++sample)
        {
            const auto mainFrequency = getCurrentFrequency();
            const auto detunedFrequency = getCurrentFrequency (getDetuneRatio());

            for (int channel = 0; channel < sampleData.getNumChannels(); ++channel)
                proxyBuffer.addSample (channel, sample, getNextSamplerSample (channel, inL, inR));

            sourceSamplePositionLeft += mainFrequency;

            if (sampleData.getNumChannels() > 1)
                sourceSamplePositionRight += detunedFrequency;

            advancePortamento();
        }

        addBufferToOutput (proxyBuffer, outputBuffer, startSample, numSamples);
    }
}

const SampleSynthSound* SampleSynthVoice::getSamplerSound (const SynthesiserSound* sound) const noexcept
{
    const auto* synthSound = dynamic_cast<const SynthSound*> (sound);

    if (synthSound == nullptr || synthSound->getVoiceType() != SynthSound::VoiceType::sampler)
        return nullptr;

    return static_cast<const SampleSynthSound*> (sound);
}

float SampleSynthVoice::readSampleAtPosition (double sourcePosition, const float* channelData) const
{
    if (sampleLength <= 1 || channelData == nullptr || sourcePosition >= static_cast<double> (sampleLength))
        return 0.0f;

    const auto pos = jlimit (0, sampleLength - interpolationLookaheadSamples, static_cast<int> (sourcePosition));
    const auto alpha = static_cast<float> (sourcePosition - pos);
    const auto invAlpha = 1.0f - alpha;

    return ((channelData[pos] * invAlpha) + (channelData[pos + 1] * alpha));
}

void SampleSynthVoice::resetPlaybackState()
{
    sourceSamplePositionLeft = 0.0;
    sourceSamplePositionRight = 0.0;
    sampleLength = 0;
}
