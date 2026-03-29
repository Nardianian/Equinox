/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Delay.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "DelayFX.h"

DelayFX::DelayFX() : delay (maximumDelay) {}

void DelayFX::prepare (const dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float> (spec.sampleRate);
    delay.prepare (spec);
}

void DelayFX::reset() { delay.reset(); }

bool DelayFX::isActive() const { return mix > 0.0f; }

void DelayFX::setParameters (float noteDivision, float feedbackAmount, float wetMix)
{
    if (approximatelyEqual (wetMix, 0.0f))
        reset();

    time = noteDivision;
    feedback = feedbackAmount * 0.01f;
    mix = wetMix;
    delay.setDelay (calculateDelayInSamples());
}

float DelayFX::calculateDelayInSamples()
{
    if (!std::isfinite (currentBPM) || currentBPM <= 0.0f)
        return 1.0f;

    const auto quarterNoteInSeconds = (minuteInSeconds / currentBPM) / 4.0f;
    const auto delayInSeconds = quarterNoteInSeconds * time;
    return jlimit (1.0f, static_cast<float> (maximumDelay - 1), delayInSeconds * sampleRate);
}

void DelayFX::process (AudioBuffer<float>& bufferToProcess, double& bpm)
{
    if (isActive())
    {
        // Checking if the currentBPM has changed
        if (std::isfinite (bpm) && bpm > 0.0 && !approximatelyEqual (currentBPM, static_cast<float> (bpm)))
        {
            // Setting the current bpm and calculating the delay
            currentBPM = static_cast<float> (bpm);
            delay.setDelay (calculateDelayInSamples());
        }

        for (int sample = 0; sample < bufferToProcess.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < bufferToProcess.getNumChannels(); ++channel)
            {
                float delayedSample = delay.popSample (channel);

                // Adding the delayed sample to the buffer
                bufferToProcess.addSample (channel, sample, delayedSample * mix * feedback);

                float nextSample = bufferToProcess.getSample (channel, sample);

                // Pushing the next buffer sample to the delay
                delay.pushSample (channel, nextSample);
            }
        }
    }
}
