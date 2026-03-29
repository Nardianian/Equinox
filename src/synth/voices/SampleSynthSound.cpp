/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    SampleSynthSound.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "SampleSynthSound.h"

SampleSynthSound::SampleSynthSound (std::shared_ptr<const AudioSample> sampleIn, int midiRootNoteIn)
    : SynthSound (VoiceType::sampler), midiRootNote (midiRootNoteIn)
{
    setSample (std::move (sampleIn));
}

void SampleSynthSound::setSample (std::shared_ptr<const AudioSample> newSample) noexcept
{
    std::atomic_store_explicit (&sample, std::move (newSample), std::memory_order_release);
}

std::shared_ptr<const AudioSample> SampleSynthSound::getSample() const noexcept
{
    return std::atomic_load_explicit (&sample, std::memory_order_acquire);
}

int SampleSynthSound::getMidiRootNote() const noexcept { return midiRootNote; }
