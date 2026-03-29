/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    VoiceEngine.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "VoiceEngine.h"

VoiceEngine::VoiceEngine (VoiceType engineVoiceType) : voiceType (engineVoiceType)
{
    heldDownNotes.ensureStorageAllocated (maxHeldNotes);
}

VoiceEngine::~VoiceEngine()
{
    clearSounds();
    clearVoices();
}

void VoiceEngine::prepare (const dsp::ProcessSpec& spec)
{
    heldDownNotes.clearQuick();
    setCurrentPlaybackSampleRate (spec.sampleRate);

    for (auto* voice : voices)
        static_cast<SynthVoice*> (voice)->prepareVoice (spec);
}

void VoiceEngine::addVoices (int voicesToAdd)
{
    clearAllCurrentNotes();
    heldDownNotes.clearQuick();
    clearVoices();

    if (voiceType == VoiceType::oscillator)
    {
        for (int i = 0; i < voicesToAdd; i++)
            addVoice (new OscSynthVoice (false));

        clearSounds();
        addSound (new OscSynthSound());
    }
    else if (voiceType == VoiceType::sampler)
    {
        for (int i = 0; i < voicesToAdd; i++)
            addVoice (new SampleSynthVoice (false));
    }

    setMonoEnabled (monophonic);
}

void VoiceEngine::setMonoEnabled (bool shouldEnableMono)
{
    const ScopedLock sl (lock);
    monophonic = shouldEnableMono;
    heldDownNotes.clearQuick();

    for (auto* voice : voices)
        static_cast<SynthVoice*> (voice)->setMonoMode (shouldEnableMono);
}

void VoiceEngine::clearAllCurrentNotes()
{
    const ScopedLock sl (lock);
    heldDownNotes.clearQuick();

    for (auto* voice : voices)
    {
        voice->setKeyDown (false);
        voice->setSustainPedalDown (false);
        voice->setSostenutoPedalDown (false);
        stopVoice (voice, 0.0f, false);
        static_cast<SynthVoice*> (voice)->resetState();
    }
}

bool VoiceEngine::hasActiveVoices() const noexcept
{
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            return true;

    return false;
}

void VoiceEngine::noteOn (const int midiChannel, const int midiNoteNumber, const float velocity)
{
    const ScopedLock sl (lock);

    if (monoEnabled())
    {
        if (heldDownNotes.isEmpty())
            return;

        const auto* monoVoice = getMonoVoice();
        const bool shouldRetrigger = monoVoice == nullptr || !monoVoice->isVoiceActive() || !monoVoice->isKeyDown();
        startMonoVoice (heldDownNotes.getLast(), shouldRetrigger);
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
            sound,
            midiChannel,
            midiNoteNumber,
            velocity);
    }
}

void VoiceEngine::noteOff (const int midiChannel,
    const int midiNoteNumber,
    const float velocity,
    const bool allowTailOff)
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
        if ((voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel (midiChannel)))
        {
            if (auto sound = voice->getCurrentlyPlayingSound())
            {
                if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
                {
                    if (!monoEnabled() || heldDownNotes.size() == 0)
                    {
                        voice->setKeyDown (false);
                    }

                    if (!(voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
                    {
                        stopVoice (voice, velocity, allowTailOff);
                    }
                }
            }
        }
    }
}

SynthesiserVoice* VoiceEngine::findFreeVoice (SynthesiserSound* soundToPlay,
    int midiChannel,
    int midiNoteNumber,
    const bool stealIfNoneAvailable) const
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

void VoiceEngine::handleMidiEvent (const MidiMessage& m)
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
            const bool releasedActiveNote =
                !heldDownNotes.isEmpty() && heldDownNotes.getLast().matches (channel, m.getNoteNumber());

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
        lastPitchWheelValues[channel - 1] = wheelPos;
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

bool VoiceEngine::monoEnabled() const { return monophonic; }

SynthVoice* VoiceEngine::getMonoVoice() const
{
    if (!monoEnabled() || voices.isEmpty())
        return nullptr;

    return static_cast<SynthVoice*> (voices.getFirst());
}

SynthesiserSound* VoiceEngine::findSoundForNote (int midiChannel, int midiNoteNumber) const
{
    for (auto* sound : sounds)
        if (sound->appliesToNote (midiNoteNumber) && sound->appliesToChannel (midiChannel))
            return sound;

    return nullptr;
}

void VoiceEngine::startMonoVoice (const MidiNote& note, bool shouldRetrigger)
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

void VoiceEngine::stopMonoVoice (float velocity, bool allowTailOff)
{
    if (auto* voice = getMonoVoice())
    {
        if (!voice->isVoiceActive())
            return;

        voice->setKeyDown (false);

        if (!(voice->isSustainPedalDown() || voice->isSostenutoPedalDown()))
            stopVoice (voice, velocity, allowTailOff);
    }
}

void VoiceEngine::addHeldNote (const MidiNote& note)
{
    removeHeldNote (note.channel, note.noteNumber);

    if (heldDownNotes.size() >= maxHeldNotes)
        heldDownNotes.remove (0);

    heldDownNotes.add (note);
}

bool VoiceEngine::removeHeldNote (int midiChannel, int midiNoteNumber)
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
