/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Filter.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "synth/VoiceParameters.h"
#include <JuceHeader.h>

/** Voice filter wrapper that owns cutoff, resonance, drive, and mode state. */
class Filter
{
public:
    /** Prepares the filter*/
    void prepareToPlay (const dsp::ProcessSpec&);

    /** Resets the filter*/
    void reset();

    /** Sets the mode of the filter*/
    void setMode (float mode);

    /** Returns true if the filter is active*/
    [[nodiscard]] bool isActive() const;

    /** Sets the filters parameters*/
    void setFilter (const VoiceParameters::Filter& filterSettings);

    /** Modulate the cutoff frequency*/
    void modulateCutoff (float cutoff);

    /** Returns the filters cutoff value*/
    [[nodiscard]] float getCutoffValue() const;

    /** Processes an AudioBuffer through the filter*/
    void process (AudioBuffer<float>&);

    /** Processes a single-sample frame through the filter. */
    void processSampleFrame (float* const* channelData, int numChannels, int sampleIndex);

    /** Restores the cutoff to the unmodulated base parameter value. */
    void restoreCutoff();

private:
    [[nodiscard]] float sanitizeCutoff (float cutoff) const noexcept;
    [[nodiscard]] float maximumCutoffHz() const noexcept;

    /** Small adapter that exposes the JUCE ladder filter's protected sample helpers. */
    struct LadderFilterProcessor : public dsp::LadderFilter<float>
    {
        using dsp::LadderFilter<float>::processSample;
        using dsp::LadderFilter<float>::updateSmoothers;
    };

    LadderFilterProcessor ladderFilter;
    double sampleRate = 44100.0;
    float currentCutoffValue = 22050.0f;
    float currentDriveValue = 1.0f;
    float currentResonanceValue = 0.0f;
    int filterMode = 0;
};
