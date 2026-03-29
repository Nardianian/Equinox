/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SampleSynthVoice.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "SampleSynthSound.h"
#include "SynthVoice.h"
#include "synth/VoiceParameters.h"

/** Voice implementation used by the sampler synth engine. */
class SampleSynthVoice : public SynthVoice
{
public:
    /** Creates a sampler voice and configures whether it should behave as mono. */
    SampleSynthVoice (bool isMonoVoice);

    ~SampleSynthVoice() override;

    /** Returns wether the sound is available*/
    bool canPlaySound (SynthesiserSound* sound) override;

    /** Starts a note*/
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override;

    /** Applies sampler-specific parameters such as sample start offset. */
    void applySamplerParameters (const VoiceParameters::Sampler& parameters);

    /** Returns the current sample start time*/
    [[nodiscard]] float getSampleStartTime() const;

    /** Returns the next  sample*/
    [[nodiscard]] float getNextSamplerSample (int channel, const float* inL, const float* inR);

    /** Renders the next block*/
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    /** Returns the typed sampler sound descriptor if the supplied sound is compatible. */
    [[nodiscard]] const SampleSynthSound* getSamplerSound (const SynthesiserSound* sound) const noexcept;

    /** Clears per-note sample playback state when the voice is reset. */
    void resetPlaybackState() override;

    /** Sets the normalized sample start offset. */
    void setSampleStartTime (float startTime);

    /** Reads a sample value at a fractional source position using interpolation. */
    [[nodiscard]] float readSampleAtPosition (double sourcePosition, const float* channelData) const;

    double sourceSamplePositionLeft = 0;
    double sourceSamplePositionRight = 0;
    double sampleStart = 0;
    int sampleLength = 0;
};
