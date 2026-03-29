/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Reverb.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "ReverbFX.h"

void ReverbFX::prepare (const dsp::ProcessSpec& spec) { reverb.prepare (spec); }

void ReverbFX::reset() { reverb.reset(); }

bool ReverbFX::isActive() const { return mix > 0.0f; }

void ReverbFX::setParameters (float roomSize, float damping, float width, float wetMix)
{
    mix = wetMix;

    if (approximatelyEqual (wetMix, 0.0f))
    {
        reset();
    }

    reverbParameters.roomSize = roomSize;
    reverbParameters.damping = damping;
    reverbParameters.width = width;
    reverbParameters.dryLevel = 0.5f - (wetMix * 0.5f);
    reverbParameters.wetLevel = wetMix;
    reverbParameters.freezeMode = 0.0f;

    reverb.setParameters (reverbParameters);
}

void ReverbFX::process (AudioBuffer<float>& bufferToProcess)
{
    if (isActive())
    {
        dsp::AudioBlock<float> inputBlock (bufferToProcess);
        reverb.process (dsp::ProcessContextReplacing<float> (inputBlock));
    }
}
