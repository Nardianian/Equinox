/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    OscSynthVoice.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "OscSynthSound.h"
#include "SynthVoice.h"
#include "synth/VoiceParameters.h"
#include "synth/dsp/WavetableOscillator.h"

/** Voice implementation used by the oscillator synth engine. */
class OscSynthVoice : public SynthVoice
{
public:
    /** Creates an oscillator voice and configures whether it should behave as mono. */
    OscSynthVoice (bool isMonoVoice);

    ~OscSynthVoice() override;

    /** Returns wether the sound is available*/
    bool canPlaySound (SynthesiserSound*) override;

    /** Starts a note*/
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override;

    /** Renders the next block*/
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    /** Applies oscillator-specific parameters such as waveform and phase. */
    void applyOscillatorParameters (const VoiceParameters::Oscillator& parameters);

    /** Sets the phase to a value between 0 and 1*/
    void setPhase (float phase);

    /** Returns the phase value*/
    [[nodiscard]] float getPhase() const;

private:
    /** Returns a pointer to the oscillator sound, or nullptr if the sound is not an oscillator sound*/
    [[nodiscard]] const OscSynthSound* getOscillatorSound (const SynthesiserSound* sound) const noexcept;

    /** Returns the next oscillator sample, based on the current set waveform*/
    double getNextOscillatorSample (int channel, double mainFrequency, double detunedFrequency);

    /** Selects the oscillator waveform for both per-voice oscillators. */
    void setWaveform (float selectedWaveform);

    float phase = 0;

    WavetableOscillator oscillator1, oscillator2;
};
