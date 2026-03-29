/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AmpEnvelope.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "AmpEnvelope.h"

void AmpEnvelope::prepareToPlay (double sampleRate) { envelope.setSampleRate (sampleRate); }

void AmpEnvelope::setEnvelope (const VoiceParameters::Envelope& envelopeSettings)
{
    envelopeParameters.attack = jmax (envelopeSettings.attack, minAttackValue);
    envelopeParameters.decay = envelopeSettings.decay;
    envelopeParameters.sustain = envelopeSettings.sustain;
    envelopeParameters.release = jmax (envelopeSettings.release, minReleaseValue);
    envelope.setParameters (envelopeParameters);
}

float AmpEnvelope::getNextSample() { return envelope.getNextSample(); }

void AmpEnvelope::setSampleRate (double sampleRate) { envelope.setSampleRate (sampleRate); }

void AmpEnvelope::noteOn() { envelope.noteOn(); }

void AmpEnvelope::noteOff() { envelope.noteOff(); }

bool AmpEnvelope::isActive() const { return envelope.isActive(); }
