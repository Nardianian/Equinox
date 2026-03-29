/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    VoiceParameters.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Per-voice parameter definitions and comparison operators. */
namespace VoiceParameters
{
    /** Compares two floating-point voice parameters using approximate equality. */
    inline bool valuesMatch (float lhs, float rhs) noexcept { return approximatelyEqual (lhs, rhs); }

    /** Common ADSR envelope settings shared by amp and filter envelopes. */
    struct Envelope
    {
        float attack = 0.0f;
        float decay = 0.0f;
        float sustain = 0.0f;
        float release = 0.0f;

        bool operator== (const Envelope& other) const noexcept
        {
            return valuesMatch (attack, other.attack) && valuesMatch (decay, other.decay)
                   && valuesMatch (sustain, other.sustain) && valuesMatch (release, other.release);
        }
    };

    /** Runtime filter settings shared by all voice types. */
    struct Filter
    {
        float mode = 0.0f;
        float cutoff = 22050.0f;
        float resonance = 0.0f;
        float drive = 1.0f;
        float envelopeCutoffLimit = 10000.0f;

        bool operator== (const Filter& other) const noexcept
        {
            return valuesMatch (mode, other.mode) && valuesMatch (cutoff, other.cutoff)
                   && valuesMatch (resonance, other.resonance) && valuesMatch (drive, other.drive)
                   && valuesMatch (envelopeCutoffLimit, other.envelopeCutoffLimit);
        }
    };

    /** Shared per-voice parameters applied to both oscillator and sampler voices. */
    struct Common
    {
        float amplitude = 0.0f;
        float detune = 0.0f;
        float finePitch = 0.0f;
        float oscPanning = 0.0f;
        float pitchTranspose = 0.0f;
        float analogValue = 0.0f;
        float portamento = 0.0f;
        Envelope ampEnvelope;
        Envelope filterEnvelope;
        Filter filter;

        bool operator== (const Common& other) const noexcept
        {
            return valuesMatch (amplitude, other.amplitude) && valuesMatch (detune, other.detune)
                   && valuesMatch (finePitch, other.finePitch) && valuesMatch (oscPanning, other.oscPanning)
                   && valuesMatch (pitchTranspose, other.pitchTranspose) && valuesMatch (analogValue, other.analogValue)
                   && valuesMatch (portamento, other.portamento) && ampEnvelope == other.ampEnvelope
                   && filterEnvelope == other.filterEnvelope && filter == other.filter;
        }
    };

    /** Oscillator-specific per-voice settings. */
    struct Oscillator
    {
        float waveform = 0.0f;
        float phase = 0.0f;

        bool operator== (const Oscillator& other) const noexcept
        {
            return valuesMatch (waveform, other.waveform) && valuesMatch (phase, other.phase);
        }
    };

    /** Sampler-specific per-voice settings. */
    struct Sampler
    {
        float sampleStartTime = 0.0f;

        bool operator== (const Sampler& other) const noexcept
        {
            return valuesMatch (sampleStartTime, other.sampleStartTime);
        }
    };

    /** Complete parameter bundle for one voice update. */
    struct Voice
    {
        Common common;
        Oscillator oscillator;
        Sampler sampler;
    };
}
