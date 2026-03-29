/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    WavetableOscillator.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "WavetableOscillator.h"
#include <cmath>

namespace
{
    constexpr auto firstWavetableWaveform = WavetableOscillator::Waveform::sineWave;
    constexpr float normalisedPhaseMaximum = 1.0f;
    constexpr double positiveFrequencyThreshold = 0.0;
    constexpr double noiseSampleScale = 2.0;
    constexpr double noiseSampleOffset = 1.0;
    constexpr float harmonicBlendScale = 0.5f;
    constexpr int firstHarmonic = 1;
    constexpr int sawHarmonicStep = 1;
    constexpr int oddHarmonicStep = 2;

    template <typename Table>
    void normalizeFrequencyTable (Table& frequencyTable)
    {
        float peakMagnitude = 0.0f;

        for (const auto sample : frequencyTable)
            peakMagnitude = jmax (peakMagnitude, std::abs (sample));

        if (peakMagnitude <= 0.0f)
            return;

        const auto inversePeakMagnitude = 1.0f / peakMagnitude;

        for (auto& sample : frequencyTable)
            sample *= inversePeakMagnitude;
    }
}

WavetableOscillator::SharedState WavetableOscillator::sharedState {};

const std::array<double, WavetableOscillator::frequencyRange> WavetableOscillator::frequencies = { 16.351599,
    17.323914,
    18.354048,
    19.445436,
    20.601723,
    21.826765,
    23.124651,
    24.5,
    25.956543,
    27.5,
    29.135235,
    30.867706,
    32.703197,
    34.647827,
    36.708096,
    38.890873,
    41.203445,
    43.65353,
    46.249302,
    49.,
    51.913086,
    55.,
    58.27047,
    61.735413,
    65.406395,
    69.295654,
    73.416191,
    77.781746,
    82.406891,
    87.30706,
    92.498604,
    97.998856,
    103.826172,
    110.,
    116.540939,
    123.470825,
    130.81279,
    138.591309,
    146.832382,
    155.563492,
    164.813782,
    174.61412,
    184.997208,
    195.997711,
    207.652344,
    220.,
    233.081879,
    246.94165,
    261.62558,
    277.182617,
    293.664764,
    311.126984,
    329.627563,
    349.228241,
    369.994415,
    391.995422,
    415.304688,
    440.,
    466.163757,
    493.883301,
    523.25116,
    554.365234,
    587.329529,
    622.253967,
    659.255127,
    698.456482,
    739.988831,
    783.990845,
    830.609375,
    880.,
    932.327515,
    987.766602,
    1046.502319,
    1108.730469,
    1174.659058,
    1244.507935,
    1318.510254,
    1396.912964,
    1479.977661,
    1567.981689,
    1661.21875,
    1760.,
    1864.655029,
    1975.533203,
    2093.004639,
    2217.460938,
    2349.318115,
    2489.015869,
    2637.020508,
    2793.825928,
    2959.955322,
    3135.963379,
    3322.4375,
    3520.,
    3729.31,
    3951.066406,
    4186.009277,
    4434.921875,
    4698.63623,
    4978.031738,
    5274.041016,
    5587.651855,
    5919.910645,
    6271.926758,
    6644.875,
    7040.,
    7458.620117,
    7902.132812,
    8372.018555,
    8869.84375,
    9397.272461,
    9956.063477,
    10548.082031,
    11175.303711,
    11839.821289,
    12543.853516,
    13289.75 };

void WavetableOscillator::prepare (double newSampleRate)
{
    const auto roundedSampleRate = static_cast<int> (std::lround (newSampleRate));

    if (roundedSampleRate <= 0)
        return;

    if (sharedState.isPrepared && sharedState.sampleRate == roundedSampleRate)
        return;

    sharedState.sampleRate = roundedSampleRate;
    sharedState.nyquistFrequency = roundedSampleRate / 2;

    generateWavetable (Waveform::sineWave);
    generateWavetable (Waveform::sawWave);
    generateWavetable (Waveform::triangleWave);
    generateWavetable (Waveform::squareWave);

    sharedState.isPrepared = true;
}

void WavetableOscillator::setWaveform (Waveform waveform)
{
    currentWaveform = waveform;
    currentWaveformTables = nullptr;
    currentFrequencyTable = nullptr;

    if (!sharedState.isPrepared || !isWavetableWaveform (waveform))
        return;

    currentWaveformTables = &getWaveformTables (waveform);

    if (currentFrequency > positiveFrequencyThreshold)
        currentFrequencyTable = &(*currentWaveformTables)[findFrequencyIndex (currentFrequency)];
}

float WavetableOscillator::getPhase() const { return phase / static_cast<float> (waveTableSize - 1); }

void WavetableOscillator::setPhase (float phaseValue)
{
    if (!isPositiveAndBelow (phaseValue, normalisedPhaseMaximum)
        && !approximatelyEqual (phaseValue, normalisedPhaseMaximum))
        return;

    phase = phaseValue * static_cast<float> (waveTableSize - 1);
}

double WavetableOscillator::getNextSample (double frequency)
{
    jassert (sharedState.isPrepared);

    if (!sharedState.isPrepared)
        return 0.0;

    if (currentWaveform == Waveform::noise)
        return nextNoiseSample();

    return nextWaveformSample (frequency);
}

bool WavetableOscillator::isWavetableWaveform (Waveform waveform) { return waveform != Waveform::noise; }

size_t WavetableOscillator::getWaveformBankIndex (Waveform waveform)
{
    jassert (isWavetableWaveform (waveform));
    return static_cast<size_t> (waveform) - static_cast<size_t> (firstWavetableWaveform);
}

WavetableOscillator::WaveformTables& WavetableOscillator::getMutableWaveformTables (Waveform waveform)
{
    return sharedState.wavetableBanks[getWaveformBankIndex (waveform)];
}

const WavetableOscillator::WaveformTables& WavetableOscillator::getWaveformTables (Waveform waveform)
{
    return sharedState.wavetableBanks[getWaveformBankIndex (waveform)];
}

size_t WavetableOscillator::findFrequencyIndex (double frequency)
{
    const auto iterator = std::lower_bound (frequencies.begin(), frequencies.end(), frequency);

    if (iterator == frequencies.end())
        return frequencies.size() - 1;

    return static_cast<size_t> (std::distance (frequencies.begin(), iterator));
}

float WavetableOscillator::getHarmonicSample (Waveform waveform, double currentAngle, int harmonic)
{
    const auto harmonicAsDouble = static_cast<double> (harmonic);

    if (waveform == Waveform::triangleWave)
        return static_cast<float> (
            std::sin (currentAngle + MathConstants<double>::halfPi) / (harmonicAsDouble * harmonicAsDouble));

    return static_cast<float> (std::sin (currentAngle) / harmonicAsDouble);
}

double WavetableOscillator::nextWaveformSample (double frequency)
{
    setFrequency (frequency);

    if (currentFrequencyTable == nullptr)
        return 0.0;

    const auto index0 = static_cast<size_t> (phase);
    const auto index1 = index0 == static_cast<size_t> (waveTableSize - 1) ? 0 : index0 + 1;
    const auto fraction = phase - static_cast<float> (index0);

    const auto value0 = (*currentFrequencyTable)[index0];
    const auto value1 = (*currentFrequencyTable)[index1];
    const auto outputSample = value0 + fraction * (value1 - value0);

    phase += phaseIncrement;

    if (phase > static_cast<float> (waveTableSize - 1))
        phase -= static_cast<float> (waveTableSize);

    return static_cast<double> (outputSample);
}

double WavetableOscillator::nextNoiseSample() const
{
    return (Random::getSystemRandom().nextDouble() * noiseSampleScale) - noiseSampleOffset;
}

void WavetableOscillator::setFrequency (double frequency)
{
    if (!sharedState.isPrepared || frequency <= positiveFrequencyThreshold)
        return;

    if (currentWaveformTables == nullptr)
    {
        if (!isWavetableWaveform (currentWaveform))
            return;

        currentWaveformTables = &getWaveformTables (currentWaveform);
    }

    const auto sampleRateChanged = preparedSampleRate != sharedState.sampleRate;

    if (!approximatelyEqual (currentFrequency, frequency) || sampleRateChanged || currentFrequencyTable == nullptr)
    {
        currentFrequency = frequency;
        preparedSampleRate = sharedState.sampleRate;
        phaseIncrement = static_cast<float> (
            currentFrequency * (static_cast<double> (waveTableSize) / static_cast<double> (preparedSampleRate)));
        currentFrequencyTable = &(*currentWaveformTables)[findFrequencyIndex (currentFrequency)];
    }
}

void WavetableOscillator::generateWavetable (Waveform waveform)
{
    jassert (isWavetableWaveform (waveform));

    auto& tables = getMutableWaveformTables (waveform);

    for (size_t frequencyIndex = 0; frequencyIndex < frequencies.size(); ++frequencyIndex)
    {
        const auto frequency = frequencies[frequencyIndex];
        auto& frequencyTable = tables[frequencyIndex];
        frequencyTable.fill (0.0f);

        int harmonic = firstHarmonic;
        auto harmonicFrequency = static_cast<double> (harmonic) * frequency;

        while (harmonicFrequency < static_cast<double> (sharedState.nyquistFrequency))
        {
            const auto angleDelta = (twoPi / static_cast<double> (waveTableSize)) * static_cast<double> (harmonic);
            auto currentAngle = 0.0;

            for (auto& sample : frequencyTable)
            {
                sample += getHarmonicSample (waveform, currentAngle, harmonic) * harmonicBlendScale;
                currentAngle += angleDelta;
            }

            if (waveform == Waveform::sineWave)
                break;

            harmonic += waveform == Waveform::sawWave ? sawHarmonicStep : oddHarmonicStep;
            harmonicFrequency = static_cast<double> (harmonic) * frequency;
        }

        normalizeFrequencyTable (frequencyTable);
    }
}
