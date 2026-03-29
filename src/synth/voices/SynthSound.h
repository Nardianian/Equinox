/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    SynthSound.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Shared sound descriptor base for Equinox oscillator and sampler voices. */
class SynthSound : public SynthesiserSound
{
public:
    /** Identifies the voice family that can render this sound. */
    enum class VoiceType { oscillator, sampler };

    /** Creates a sound descriptor for the supplied voice family. */
    explicit SynthSound (VoiceType voiceTypeToUse) : voiceType (voiceTypeToUse) { midiNoteRange.setRange (0, 128, true); }

    ~SynthSound() override = default;

    /** Returns the concrete voice family that can render this sound. */
    [[nodiscard]] VoiceType getVoiceType() const noexcept { return voiceType; }

    /** Returns whether the sound should respond to the supplied MIDI note. */
    bool appliesToNote (int midiNoteNumber) override { return midiNoteRange[midiNoteNumber]; }

    /** Returns whether the sound should respond to the supplied MIDI channel. */
    bool appliesToChannel (int midiChannel) override
    {
        ignoreUnused (midiChannel);
        return true;
    }

protected:
    /** MIDI note mask used by JUCE when matching sounds for note-on events. */
    BigInteger midiNoteRange;

private:
    VoiceType voiceType;
};
