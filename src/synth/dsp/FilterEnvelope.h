/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    FilterEnvelope.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "synth/dsp/ADSREnvelope.h"
#include "synth/dsp/Filter.h"

/** Envelope that modulates a voice filter cutoff over the lifetime of a note. */
class FilterEnvelope : public ADSREnvelope
{
public:
    /** Creates a filter envelope that drives the supplied filter instance. */
    FilterEnvelope (Filter&);

    /** Prepares the filter envelope*/
    void prepareToPlay (double sampleRate) override;

    /** Sets the filter envelopes parameters*/
    void setEnvelope (const VoiceParameters::Envelope& envelopeSettings) override;

    /** Sets the cutoff limit*/
    void setCutoffLimit (float cutoffLimit);

    /** Should be called when a note is triggered*/
    void noteOn() override;

    /** Should be called when a note is released*/
    void noteOff() override;

    /** Returns true if the filter envelope is currently active*/
    [[nodiscard]] bool isActive() const override;

    /** Calculates the filter envelopes next value*/
    void calculateNextValue();

private:
    bool enabled = false;

    Filter& filter;

    float cutoffUpperLimit = 0;

    float cutoffLowerLimit = 0;

    bool wasActive = false;
};
