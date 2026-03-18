/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Synth.h
    Author:  Oliver Rasmussen
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OscSynthVoice.h"
#include "SampleSynthVoice.h"
class Synth : public Synthesiser
{
public:
    
    enum voiceType {oscillator, sampler};
    
    Synth(voiceType synthVoiceType);
    
    ~Synth();
    
    void prepare(dsp::ProcessSpec& spec);
    
    void addVoices(int voicesToAdd);
    
    void clearAllCurrentNotes();
    
protected:
    /** Method for finding a free voice*/
    SynthesiserVoice* findFreeVoice (SynthesiserSound* soundToPlay, int midiChannel, int midiNoteNumber, const bool stealIfNoneAvailable) const override;
    
    /** Method that gets called when a midi note is triggered*/
    void noteOn (const int midiChannel, const int midiNoteNumber, const float velocity) override;
    
    /** Method that gets called when a midi note is released*/
    void noteOff (const int midiChannel, const int midiNoteNumber, const float velocity, const bool allowTailOff) override;
    
    /** Method used for handling midi events*/
    void handleMidiEvent(const MidiMessage &) override;
    
private:
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

        int channel;
        int noteNumber;
        float velocity;
    };

    /** Returns true if the synth currently only has one voice*/
    bool monoEnabled() const;

    SynthVoice* getMonoVoice() const;
    SynthesiserSound* findSoundForNote (int midiChannel, int midiNoteNumber) const;
    void startMonoVoice (const MidiNote& note, bool shouldRetrigger);
    void stopMonoVoice (float velocity, bool allowTailOff);
    void addHeldNote (const MidiNote& note);
    bool removeHeldNote (int midiChannel, int midiNoteNumber);

    Array<MidiNote> heldDownNotes;

    voiceType _voiceType;
};
