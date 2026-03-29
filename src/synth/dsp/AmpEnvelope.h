/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AmpEnvelope.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "synth/dsp/ADSREnvelope.h"

/** Voice amplitude envelope wrapper around JUCE's ADSR. */
class AmpEnvelope : public ADSREnvelope
{
public:
    /** Prepares the amp envelope*/
    void prepareToPlay (double sampleRate) override;

    /** Sets the envelopes parameters*/
    void setEnvelope (const VoiceParameters::Envelope& envelopeSettings) override;

    /** Returns the next amp envelope sample*/
    float getNextSample();

    /** Sets the samplerate of the amp envelope*/
    void setSampleRate (double sampleRate);

    /** Should be called when a note is triggered*/
    void noteOn() override;

    /** Should be called when a note is released*/
    void noteOff() override;

    /** Returns true if the amp envelope is active*/
    [[nodiscard]] bool isActive() const override;

private:
    const float minAttackValue = 0.001f;
    const float minReleaseValue = 0.001f;
};
