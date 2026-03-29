/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    ParameterIds.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Centralized collection of all parameter IDs */
namespace ParameterIds
{
    /** Appends a synth-layer instance number to a parameter ID base name. */
    inline String withInstance (StringRef baseId, int instanceNumber)
    {
        return String (baseId) + String (instanceNumber);
    }

    /** Collection of parameter IDs for one synth layer instance. */
    struct Synth
    {
        /** Creates the parameter ID set for the supplied synth-layer instance. */
        explicit Synth (int instanceNumber)
            : ampAttack (withInstance ("ampAttack", instanceNumber)),
              ampDecay (withInstance ("ampDecay", instanceNumber)),
              ampSustain (withInstance ("ampSustain", instanceNumber)),
              ampRelease (withInstance ("ampRelease", instanceNumber)),
              filterAttack (withInstance ("filterAttack", instanceNumber)),
              filterDecay (withInstance ("filterDecay", instanceNumber)),
              filterSustain (withInstance ("filterSustain", instanceNumber)),
              filterRelease (withInstance ("filterRelease", instanceNumber)),
              envelopeCutoffLimit (withInstance ("envelopeCutoffLimit", instanceNumber)),
              waveform (withInstance ("waveform", instanceNumber)),
              isActive (withInstance ("isActive", instanceNumber)),
              amplitude (withInstance ("amplitude", instanceNumber)),
              detune (withInstance ("detune", instanceNumber)),
              finePitch (withInstance ("finePitch", instanceNumber)),
              oscPanning (withInstance ("oscPanning", instanceNumber)),
              pitchTranspose (withInstance ("pitchTranspose", instanceNumber)),
              analogValue (withInstance ("analogValue", instanceNumber)),
              filterType (withInstance ("filterType", instanceNumber)),
              cutoff (withInstance ("cutoff", instanceNumber)),
              resonance (withInstance ("resonance", instanceNumber)),
              drive (withInstance ("drive", instanceNumber)),
              synthMode (withInstance ("synthMode", instanceNumber)),
              monoEnabled (withInstance ("monoEnabled", instanceNumber)),
              portamento (withInstance ("portamento", instanceNumber)),
              phase (withInstance ("phase", instanceNumber)),
              sampleStartTime (withInstance ("sampleStartTime", instanceNumber)),
              sampleStateId (withInstance ("audiosample", instanceNumber))
        {
        }

        /** Returns all APVTS parameter IDs that belong to this synth layer. */
        [[nodiscard]] StringArray allParameterIds() const
        {
            return { ampAttack,
                ampDecay,
                ampSustain,
                ampRelease,
                filterAttack,
                filterDecay,
                filterSustain,
                filterRelease,
                envelopeCutoffLimit,
                waveform,
                isActive,
                amplitude,
                detune,
                finePitch,
                oscPanning,
                pitchTranspose,
                analogValue,
                filterType,
                cutoff,
                resonance,
                drive,
                synthMode,
                monoEnabled,
                portamento,
                phase,
                sampleStartTime };
        }

        String ampAttack;
        String ampDecay;
        String ampSustain;
        String ampRelease;
        String filterAttack;
        String filterDecay;
        String filterSustain;
        String filterRelease;
        String envelopeCutoffLimit;
        String waveform;
        String isActive;
        String amplitude;
        String detune;
        String finePitch;
        String oscPanning;
        String pitchTranspose;
        String analogValue;
        String filterType;
        String cutoff;
        String resonance;
        String drive;
        String synthMode;
        String monoEnabled;
        String portamento;
        String phase;
        String sampleStartTime;
        String sampleStateId;
    };

    namespace MasterFX
    {
        static constexpr const char* chorusMix = "chorusMix";
        static constexpr const char* chorusRate = "chorusRate";
        static constexpr const char* chorusDepth = "chorusDepth";
        static constexpr const char* chorusDelay = "chorusDelay";
        static constexpr const char* chorusFeedback = "chorusFeedback";

        static constexpr const char* phaserMix = "phaserMix";
        static constexpr const char* phaserRate = "phaserRate";
        static constexpr const char* phaserDepth = "phaserDepth";
        static constexpr const char* phaserFrequency = "phaserFrequency";
        static constexpr const char* phaserFeedback = "phaserFeedback";

        static constexpr const char* reverbMix = "reverbMix";
        static constexpr const char* reverbRoomsize = "reverbRoomsize";
        static constexpr const char* reverbDamping = "reverbDamping";
        static constexpr const char* reverbWidth = "reverbWidth";

        static constexpr const char* delayMix = "delayMix";
        static constexpr const char* delayTime = "delayTime";
        static constexpr const char* delayFeedback = "delayFeedback";

        /** Returns all APVTS parameter IDs used by the master effect chain. */
        inline StringArray allParameterIds()
        {
            return { chorusMix,
                chorusRate,
                chorusDepth,
                chorusDelay,
                chorusFeedback,
                phaserMix,
                phaserRate,
                phaserDepth,
                phaserFrequency,
                phaserFeedback,
                reverbMix,
                reverbRoomsize,
                reverbDamping,
                reverbWidth,
                delayMix,
                delayTime,
                delayFeedback };
        }
    } // namespace MasterFX

    /** Returns every APVTS parameter ID used by the plugin. */
    inline StringArray allParameterIds (int synthLayerCount)
    {
        StringArray ids;

        for (int instanceNumber = 1; instanceNumber <= synthLayerCount; ++instanceNumber)
            ids.addArray (Synth (instanceNumber).allParameterIds());

        ids.addArray (MasterFX::allParameterIds());
        return ids;
    }
} // namespace ParameterIds
