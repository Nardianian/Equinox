/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    VoiceEngine.h
    Author:  Oliver Rasmussen
  ==============================================================================
*/

#pragma once

#include "synth/voices/OscSynthVoice.h"
#include "synth/voices/SampleSynthVoice.h"
#include <JuceHeader.h>

/** Thin JUCE synthesiser wrapper that adds Equinox-specific voice setup and mono note-stack behavior. */
class VoiceEngine : public Synthesiser
{
public:
    /** Identifies the concrete voice family owned by the engine. */
    enum class VoiceType { oscillator, sampler };
    static constexpr int maxHeldNotes = 32;

    /** Creates an engine that will instantiate the requested voice type. */
    explicit VoiceEngine (VoiceType voiceType);
    /** Destroys the engine and owned voices/sounds. */
    ~VoiceEngine() override;

    /** Prepares all voices for playback. */
    void prepare (const dsp::ProcessSpec& spec);

    /** Adds the requested number of voices of the configured voice type. */
    void addVoices (int voicesToAdd);
    /** Enables or disables mono behavior without rebuilding voices. */
    void setMonoEnabled (bool shouldEnableMono);

    /** Force-stops all voices and clears mono held-note state. */
    void clearAllCurrentNotes();
    /** Returns whether any owned voice is currently active. */
    bool hasActiveVoices() const noexcept;

protected:
    /** Method for finding a free voice*/
    SynthesiserVoice* findFreeVoice (SynthesiserSound* soundToPlay,
        int midiChannel,
        int midiNoteNumber,
        const bool stealIfNoneAvailable) const override;

    /** Method that gets called when a midi note is triggered*/
    void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity) override;

    /** Method that gets called when a midi note is released*/
    void
        noteOff (const int midiChannel, const int midiNoteNumber, const float velocity, const bool allowTailOff) override;

    /** Method used for handling midi events*/
    void handleMidiEvent (const MidiMessage&) override;

private:
    /** Lightweight held-note entry used for mono note-stack handling. */
    struct MidiNote
    {
        MidiNote() = default;

        MidiNote (const int channelIn, const int noteNumberIn, const float velocityIn)
            : channel (channelIn), noteNumber (noteNumberIn), velocity (velocityIn)
        {
        }

        bool matches (const int midiChannel, const int midiNoteNumber) const
        {
            return channel == midiChannel && noteNumber == midiNoteNumber;
        }

        int channel = 1;
        int noteNumber = 0;
        float velocity = 0.0f;
    };

    /** Returns whether mono behavior is currently enabled. */
    bool monoEnabled() const;

    /** Returns the single voice used for mono playback. */
    SynthVoice* getMonoVoice() const;
    /** Returns the sound descriptor that should respond to the supplied note. */
    SynthesiserSound* findSoundForNote (int midiChannel, int midiNoteNumber) const;
    /** Starts or retunes the mono voice using last-note-priority behavior. */
    void startMonoVoice (const MidiNote& note, bool shouldRetrigger);
    /** Stops the mono voice using the requested tail-off mode. */
    void stopMonoVoice (float velocity, bool allowTailOff);
    /** Pushes a held note onto the mono note stack. */
    void addHeldNote (const MidiNote& note);
    /** Removes a held note from the mono note stack. */
    bool removeHeldNote (int midiChannel, int midiNoteNumber);

    Array<MidiNote> heldDownNotes;

    VoiceType voiceType;
    bool monophonic = false;
};
