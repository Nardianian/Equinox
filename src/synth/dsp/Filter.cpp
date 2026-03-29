/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Filter.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "Filter.h"

namespace
{
}

void Filter::prepareToPlay (const dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    ladderFilter.prepare (spec);
    setMode (static_cast<float> (filterMode));
    ladderFilter.setCutoffFrequencyHz (sanitizeCutoff (currentCutoffValue));
    ladderFilter.setResonance (currentResonanceValue);
    ladderFilter.setDrive (currentDriveValue);
}

void Filter::reset() { ladderFilter.reset(); }

void Filter::setMode (float mode)
{
    const auto newMode = roundToInt (mode);
    if (filterMode != newMode)
    {
        filterMode = newMode;
        switch (filterMode)
        {
            case 0:
                ladderFilter.setMode (dsp::LadderFilter<float>::Mode::LPF12);
                break;
            case 1:
                ladderFilter.setMode (dsp::LadderFilter<float>::Mode::HPF12);
                break;
            case 2:
                ladderFilter.setMode (dsp::LadderFilter<float>::Mode::LPF24);
                break;
            case 3:
                ladderFilter.setMode (dsp::LadderFilter<float>::Mode::HPF24);
                break;
            default:
                ladderFilter.setMode (dsp::LadderFilter<float>::Mode::LPF12);
                break;
        }
    }
}

bool Filter::isActive() const
{
    return ((filterMode == 0 || filterMode == 2) ? currentCutoffValue < 22050 : currentCutoffValue > 5)
           || currentDriveValue > 1 || currentResonanceValue > 0;
}

void Filter::setFilter (const VoiceParameters::Filter& filterSettings)
{
    currentCutoffValue = sanitizeCutoff (filterSettings.cutoff);
    ladderFilter.setCutoffFrequencyHz (currentCutoffValue);
    currentResonanceValue = jlimit (0.0f, 1.0f, filterSettings.resonance);
    ladderFilter.setResonance (currentResonanceValue);
    currentDriveValue = jmax (1.0f, filterSettings.drive);
    ladderFilter.setDrive (currentDriveValue);
}

void Filter::modulateCutoff (float cutoff) { ladderFilter.setCutoffFrequencyHz (sanitizeCutoff (cutoff)); }

float Filter::getCutoffValue() const { return currentCutoffValue; }

void Filter::process (AudioBuffer<float>& bufferToProcess)
{
    if (isActive())
    {
        dsp::AudioBlock<float> block (bufferToProcess);
        ladderFilter.process (dsp::ProcessContextReplacing<float> (block));
    }
}

void Filter::processSampleFrame (float* const* channelData, int numChannels, int sampleIndex)
{
    if (isActive())
    {
        ladderFilter.updateSmoothers();

        for (int channel = 0; channel < numChannels; ++channel)
            channelData[channel][sampleIndex] =
                ladderFilter.processSample (channelData[channel][sampleIndex], static_cast<size_t> (channel));
    }
}

void Filter::restoreCutoff() { ladderFilter.setCutoffFrequencyHz (sanitizeCutoff (currentCutoffValue)); }

float Filter::sanitizeCutoff (float cutoff) const noexcept { return jlimit (5.0f, maximumCutoffHz(), cutoff); }

float Filter::maximumCutoffHz() const noexcept { return jmax (5.0f, static_cast<float> ((sampleRate * 0.5) - 1.0)); }
