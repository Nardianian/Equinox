/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    OscSynthSound.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "SynthSound.h"

/** Immutable sound descriptor used by oscillator voices. */
class OscSynthSound : public SynthSound
{
public:
    /** Creates an oscillator sound descriptor. */
    OscSynthSound();
};
