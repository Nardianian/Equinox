/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Synth.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "Synth.h"

Synth::Synth(voiceType synthVoiceType)
{
    _voiceType = synthVoiceType;
}

Synth::~Synth()
{
    clearSounds();
    clearVoices();
}

void Synth::prepare(dsp::ProcessSpec& spec)
{
    heldDownNotes.clear();
    setCurrentPlaybackSampleRate(spec.sampleRate);
    
    for (auto* voice : voices)
    {
        if (auto* v = dynamic_cast<SynthVoice*>(voice))
        {
            v->prepareVoice(spec);
        }
    }
}

void Synth::addVoices(int voicesToAdd)
{
    clearAllCurrentNotes();
    heldDownNotes.clear();
    clearVoices();
    
    bool monophonic = voicesToAdd == 1 ? true : false;
    
    if (_voiceType == voiceType::oscillator)
    {
        for (int i = 0; i < voicesToAdd; i++)
        {
            addVoice(new OscSynthVoice(monophonic));
        }
        clearSounds();
        addSound(new OscSynthSound());
    }
    else if (_voiceType == voiceType::sampler)
    {
        for (int i = 0; i < voicesToAdd; i++)
        {
            addVoice(new SampleSynthVoice(monophonic));
        }
    }
}

void Synth::clearAllCurrentNotes()
{
    heldDownNotes.clear();

    for (auto* voice : voices)
    {
        if (auto* v = dynamic_cast<SynthVoice*>(voice))
        {
            v->clearCurrentNote();
        }
    }
}

void Synth::noteOn (const int midiChannel, const int midiNoteNumber, const float velocity)
{
    const ScopedLock sl (lock);

    if (monoEnabled())
    {
        if (heldDownNotes.isEmpty())
            return;

        startMonoVoice (heldDownNotes.getLast(), heldDownNotes.size() == 1);
        return;
    }

    for (auto* sound : sounds)
    {
        if (!sound->appliesToNote (midiNoteNumber) || !sound->appliesToChannel (midiChannel))
            continue;

        for (auto* voice : voices)
            if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel (midiChannel))
                stopVoice (voice, 1.0f, true);

        startVoice (findFreeVoice (sound, midiChannel, midiNoteNumber, isNoteStealingEnabled()),
                    sound, midiChannel, midiNoteNumber, velocity);
    }
}

void Synth::noteOff (const int midiChannel, const int midiNoteNumber, const float velocity, const bool allowTailOff)
{
    const ScopedLock sl (lock);

    if (monoEnabled())
    {
        if (heldDownNotes.isEmpty())
            stopMonoVoice (velocity, allowTailOff);

        return;
    }

    for (auto* voice : voices)
    {
        if ((voice->getCurrentlyPlayingNote() == midiNoteNumber
              && voice->isPlayingChannel (midiChannel)))
        {
            if (auto sound = voice->getCurrentlyPlayingSound())
            {
                if (sound->appliesToNote (midiNoteNumber)
                     && sound->appliesToChannel (midiChannel))
                {

                    if (!monoEnabled() || heldDownNotes.size() == 0)
                    {
                        voice->setKeyDown (false);
                    }

                    if (! (voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
                    {
                        stopVoice (voice, velocity, allowTailOff);
                    }
                }
            }
        }
    }
}

SynthesiserVoice* Synth::findFreeVoice (SynthesiserSound* soundToPlay, int midiChannel, int midiNoteNumber, const bool stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);

    for (auto* voice : voices)
        if (((!voice->isVoiceActive()) && voice->canPlaySound (soundToPlay)) || monoEnabled())
            return voice;

    if (stealIfNoneAvailable)
        return findVoiceToSteal (soundToPlay, midiChannel, midiNoteNumber);

    return nullptr;
}

/**
 void monoNoteOn (midiNoteNumber, velocity)
 // currentlyPlayingNote = midiNoteNumber
 // - set velocity =
 */

void Synth::handleMidiEvent(const MidiMessage& m)
{
    const int channel = m.getChannel();
    
    if (m.isNoteOn())
    {
        if (monoEnabled())
            addHeldNote ({ channel, m.getNoteNumber(), m.getFloatVelocity() });

        noteOn (channel, m.getNoteNumber(), m.getFloatVelocity());
    }
    else if (m.isNoteOff())
    {
        if (monoEnabled())
        {
            const bool releasedActiveNote = !heldDownNotes.isEmpty()
                                         && heldDownNotes.getLast().matches (channel, m.getNoteNumber());

            if (!removeHeldNote (channel, m.getNoteNumber()))
                return;

            if (releasedActiveNote && !heldDownNotes.isEmpty())
            {
                noteOn (heldDownNotes.getLast().channel,
                        heldDownNotes.getLast().noteNumber,
                        heldDownNotes.getLast().velocity);
                return;
            }
        }

        noteOff (channel, m.getNoteNumber(), m.getFloatVelocity(), true);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        heldDownNotes.clearQuick();
        allNotesOff (channel, true);
    }
    else if (m.isPitchWheel())
    {
        const int wheelPos = m.getPitchWheelValue();
        lastPitchWheelValues [channel - 1] = wheelPos;
        handlePitchWheel (channel, wheelPos);
    }
    else if (m.isAftertouch())
    {
        if (monoEnabled())
        {
            if (auto* voice = getMonoVoice())
                if (!heldDownNotes.isEmpty() && voice->isPlayingChannel (channel))
                    voice->aftertouchChanged (m.getAfterTouchValue());

            return;
        }

        handleAftertouch (channel, m.getNoteNumber(), m.getAfterTouchValue());
    }
    else if (m.isChannelPressure())
    {
        handleChannelPressure (channel, m.getChannelPressureValue());
    }
    else if (m.isController())
    {
        handleController (channel, m.getControllerNumber(), m.getControllerValue());
    }
    else if (m.isProgramChange())
    {
        handleProgramChange (channel, m.getProgramChangeNumber());
    }
}

bool Synth::monoEnabled() const
{
    return getNumVoices() == 1;
}

SynthVoice* Synth::getMonoVoice() const
{
    if (!monoEnabled() || voices.isEmpty())
        return nullptr;

    return dynamic_cast<SynthVoice*> (voices.getFirst());
}

SynthesiserSound* Synth::findSoundForNote (int midiChannel, int midiNoteNumber) const
{
    for (auto* sound : sounds)
        if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
            return sound;

    return nullptr;
}

void Synth::startMonoVoice (const MidiNote& note, bool shouldRetrigger)
{
    auto* voice = getMonoVoice();
    auto* sound = findSoundForNote (note.channel, note.noteNumber);

    if (voice == nullptr || sound == nullptr)
        return;

    if (shouldRetrigger || !voice->isVoiceActive())
    {
        startVoice (voice, sound, note.channel, note.noteNumber, note.velocity);
        return;
    }

    voice->setKeyDown (true);
    voice->startNote (note.noteNumber, note.velocity, sound, lastPitchWheelValues[note.channel - 1]);
}

void Synth::stopMonoVoice (float velocity, bool allowTailOff)
{
    if (auto* voice = getMonoVoice())
    {
        if (!voice->isVoiceActive())
            return;

        voice->setKeyDown (false);

        if (! (voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
            stopVoice (voice, velocity, allowTailOff);
    }
}

void Synth::addHeldNote (const MidiNote& note)
{
    removeHeldNote (note.channel, note.noteNumber);
    heldDownNotes.add (note);
}

bool Synth::removeHeldNote (int midiChannel, int midiNoteNumber)
{
    for (int i = heldDownNotes.size(); --i >= 0;)
    {
        if (heldDownNotes.getReference (i).matches (midiChannel, midiNoteNumber))
        {
            heldDownNotes.remove (i);
            return true;
        }
    }

    return false;
}
