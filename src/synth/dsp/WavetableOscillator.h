/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    WavetableOscillator.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <numbers>

/** Shared wavetable oscillator used by oscillator voices. */
class WavetableOscillator
{
public:
    /** Available oscillator waveforms. */
    enum class Waveform { sineWave, sawWave, triangleWave, squareWave, noise };

    WavetableOscillator() = default;
    ~WavetableOscillator() = default;

    /** Sets samplerate and generates wavetables */
    static void prepare (double sampleRate);

    /** Sets the waveform to be used */
    void setWaveform (Waveform waveform);

    /** Returns the current phase value */
    [[nodiscard]] float getPhase() const;

    /** Sets the oscillators phase. phaseValue must be between 0 and 1 */
    void setPhase (float phaseValue);

    /** Returns the next oscillator sample*/
    [[nodiscard]] double getNextSample (double frequency);

private:
    using FrequencyTable = std::array<float, 2048>;
    using WaveformTables = std::array<FrequencyTable, 117>;

    static constexpr int frequencyRange = 117;
    static constexpr int waveTableSize = 2048;
    static constexpr double pi = std::numbers::pi;
    static constexpr double twoPi = std::numbers::pi * 2.0;

    /** Shared wavetable bank and sample-rate state reused by all oscillator instances. */
    struct SharedState
    {
        bool isPrepared = false;
        int sampleRate = 0;
        int nyquistFrequency = 0;
        std::array<WaveformTables, 4> wavetableBanks {};
    };

    static SharedState sharedState;
    static const std::array<double, frequencyRange> frequencies;

    float phase = 0.0f;
    double currentFrequency = 0.0;
    float phaseIncrement = 0.0f;
    int preparedSampleRate = 0;
    Waveform currentWaveform = Waveform::sineWave;
    const WaveformTables* currentWaveformTables = nullptr;
    const FrequencyTable* currentFrequencyTable = nullptr;

    static bool isWavetableWaveform (Waveform waveform);
    static size_t getWaveformBankIndex (Waveform waveform);
    static WaveformTables& getMutableWaveformTables (Waveform waveform);
    static const WaveformTables& getWaveformTables (Waveform waveform);
    static size_t findFrequencyIndex (double frequency);
    static void generateWavetable (Waveform waveform);
    static float getHarmonicSample (Waveform waveform, double currentAngle, int harmonic);

    /** Calculates and returns the next sample,
    based on the current waveform, frequency, and phase*/
    double nextWaveformSample (double frequency);

    /** Generates random noise */
    [[nodiscard]] double nextNoiseSample() const;

    /** Set the frequency of the waveform*/
    void setFrequency (double frequency);
};
