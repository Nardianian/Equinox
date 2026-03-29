/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Phaser.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "PhaserFX.h"

void PhaserFX::prepare (const dsp::ProcessSpec& spec) { phaser.prepare (spec); }

void PhaserFX::reset() { phaser.reset(); }

bool PhaserFX::isActive() const { return mix > 0.0f; }

void PhaserFX::setParameters (float rate, float depth, float frequency, float feedback, float wetMix)
{
    if (approximatelyEqual (wetMix, 0.0f))
    {
        reset();
    }

    phaser.setRate (rate);
    phaser.setDepth (depth);
    phaser.setCentreFrequency (frequency);
    phaser.setFeedback (feedback);
    mix = wetMix;
    phaser.setMix (wetMix);
}

void PhaserFX::process (AudioBuffer<float>& bufferToProcess)
{
    if (isActive())
    {
        dsp::AudioBlock<float> inputBlock (bufferToProcess);
        phaser.process (dsp::ProcessContextReplacing<float> (inputBlock));
    }
}
