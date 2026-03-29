/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    FilterEnvelope.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "FilterEnvelope.h"

FilterEnvelope::FilterEnvelope (Filter& _filter) : filter (_filter) {}

void FilterEnvelope::prepareToPlay (double sampleRate) { envelope.setSampleRate (sampleRate); }

void FilterEnvelope::setEnvelope (const VoiceParameters::Envelope& envelopeSettings)
{
    envelopeParameters.attack = envelopeSettings.attack;
    envelopeParameters.decay = envelopeSettings.decay;
    envelopeParameters.sustain = envelopeSettings.sustain;
    envelopeParameters.release = envelopeSettings.release;
    envelope.setParameters (envelopeParameters);

    if (approximatelyEqual (envelopeParameters.attack, 0.0f) && approximatelyEqual (envelopeParameters.decay, 0.0f)
        && approximatelyEqual (envelopeParameters.sustain, 0.0f)
        && approximatelyEqual (envelopeParameters.release, 0.0f))
    {
        envelope.reset();
        filter.restoreCutoff();
        wasActive = false;
        enabled = false;
    }
    else
    {
        enabled = true;
    }
}

void FilterEnvelope::setCutoffLimit (float cutoffLimit) { cutoffUpperLimit = cutoffLimit; }

void FilterEnvelope::noteOn()
{
    if (enabled)
    {
        cutoffLowerLimit = filter.getCutoffValue();
        filter.restoreCutoff();
        envelope.noteOn();
        wasActive = false;
    }
}

void FilterEnvelope::noteOff()
{
    if (enabled)
    {
        envelope.noteOff();
    }
}

bool FilterEnvelope::isActive() const { return envelope.isActive() && enabled; }

void FilterEnvelope::calculateNextValue()
{
    if (isActive())
    {
        float nextCutoffValue = ((cutoffUpperLimit - cutoffLowerLimit) * envelope.getNextSample()) + cutoffLowerLimit;
        filter.modulateCutoff (nextCutoffValue);
        wasActive = true;
    }
    else if (wasActive)
    {
        filter.restoreCutoff();
        wasActive = false;
    }
}
