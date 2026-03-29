/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SampleSynthSound.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "SynthSound.h"
#include "state/AudioSample.h"

/** Sound descriptor for sampler voices, carrying the shared sample payload and root note. */
class SampleSynthSound : public SynthSound
{
public:
    /** Creates a sampler sound descriptor with an optional prepared sample payload. */
    explicit SampleSynthSound (std::shared_ptr<const AudioSample> sample = nullptr, int midiRootNote = 60);

    ~SampleSynthSound() override = default;

    /** Swaps in a new immutable sample payload for future voice starts. */
    void setSample (std::shared_ptr<const AudioSample> newSample) noexcept;
    /** Returns the current immutable sample payload. */
    [[nodiscard]] std::shared_ptr<const AudioSample> getSample() const noexcept;
    /** Returns the MIDI root note used for pitch-tracking the sample. */
    [[nodiscard]] int getMidiRootNote() const noexcept;

private:
    const int midiRootNote;
    std::shared_ptr<const AudioSample> sample;
};
