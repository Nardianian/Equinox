/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Delay.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DelayFX
{
public:
    DelayFX();
    ~DelayFX() = default;

    /** Prepares the delay*/
    void prepare (const dsp::ProcessSpec& spec);

    /** Resets the delay*/
    void reset();

    /** Returns true if the delay is active*/
    [[nodiscard]] bool isActive() const;

    /** Sets the delays parameters*/
    void setParameters (float noteDivision, float feedbackAmount, float wetMix);

    /** Processes the AudioBuffer reference through the delay*/
    void process (AudioBuffer<float>& bufferToProcess, double& bpm);

private:
    /** Calculates the delay in samples*/
    float calculateDelayInSamples();

    static constexpr float minuteInSeconds = 60.0f;
    static constexpr int maximumDelay = 480000;

    dsp::DelayLine<float> delay;
    float feedback = 0;
    float time = 0;
    float mix = 0;
    float currentBPM = 120;

    float sampleRate = 44100.0f;
};
