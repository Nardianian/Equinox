/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    ParameterModels.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "ParameterIds.h"

/** Typed references to parameter values and helper functions for parameter management. */
namespace ParameterModels
{
    /** Returns a raw parameter atomic or a dummy fallback if the parameter is missing. */
    inline std::atomic<float>& requireParameter (AudioProcessorValueTreeState& parameters, const String& parameterId)
    {
        if (auto* value = parameters.getRawParameterValue (parameterId))
            return *value;

        jassertfalse;
        static std::atomic<float> missingParameter { 0.0f };
        return missingParameter;
    }

    /** Compares two floating-point parameter values using JUCE's approximate equality helper. */
    inline bool floatsMatch (float lhs, float rhs) noexcept { return approximatelyEqual (lhs, rhs); }

    /** Typed references to all synth-layer parameters for one layer instance. */
    struct SynthParameters
    {
        explicit SynthParameters (AudioProcessorValueTreeState& parameters, int instanceNumber)
            : ids (instanceNumber),
              ampAttack (requireParameter (parameters, ids.ampAttack)),
              ampDecay (requireParameter (parameters, ids.ampDecay)),
              ampSustain (requireParameter (parameters, ids.ampSustain)),
              ampRelease (requireParameter (parameters, ids.ampRelease)),
              filterAttack (requireParameter (parameters, ids.filterAttack)),
              filterDecay (requireParameter (parameters, ids.filterDecay)),
              filterSustain (requireParameter (parameters, ids.filterSustain)),
              filterRelease (requireParameter (parameters, ids.filterRelease)),
              envelopeCutoffLimit (requireParameter (parameters, ids.envelopeCutoffLimit)),
              waveform (requireParameter (parameters, ids.waveform)),
              isActive (requireParameter (parameters, ids.isActive)),
              amplitude (requireParameter (parameters, ids.amplitude)),
              detune (requireParameter (parameters, ids.detune)),
              finePitch (requireParameter (parameters, ids.finePitch)),
              oscPanning (requireParameter (parameters, ids.oscPanning)),
              pitchTranspose (requireParameter (parameters, ids.pitchTranspose)),
              analogValue (requireParameter (parameters, ids.analogValue)),
              filterType (requireParameter (parameters, ids.filterType)),
              cutoff (requireParameter (parameters, ids.cutoff)),
              resonance (requireParameter (parameters, ids.resonance)),
              drive (requireParameter (parameters, ids.drive)),
              synthMode (requireParameter (parameters, ids.synthMode)),
              monoEnabled (requireParameter (parameters, ids.monoEnabled)),
              portamento (requireParameter (parameters, ids.portamento)),
              phase (requireParameter (parameters, ids.phase)),
              sampleStartTime (requireParameter (parameters, ids.sampleStartTime))
        {
        }

        ParameterIds::Synth ids;
        std::atomic<float>& ampAttack;
        std::atomic<float>& ampDecay;
        std::atomic<float>& ampSustain;
        std::atomic<float>& ampRelease;
        std::atomic<float>& filterAttack;
        std::atomic<float>& filterDecay;
        std::atomic<float>& filterSustain;
        std::atomic<float>& filterRelease;
        std::atomic<float>& envelopeCutoffLimit;
        std::atomic<float>& waveform;
        std::atomic<float>& isActive;
        std::atomic<float>& amplitude;
        std::atomic<float>& detune;
        std::atomic<float>& finePitch;
        std::atomic<float>& oscPanning;
        std::atomic<float>& pitchTranspose;
        std::atomic<float>& analogValue;
        std::atomic<float>& filterType;
        std::atomic<float>& cutoff;
        std::atomic<float>& resonance;
        std::atomic<float>& drive;
        std::atomic<float>& synthMode;
        std::atomic<float>& monoEnabled;
        std::atomic<float>& portamento;
        std::atomic<float>& phase;
        std::atomic<float>& sampleStartTime;
    };

    /** Typed references to all master-effect parameters. */
    struct MasterEffectParameters
    {
        explicit MasterEffectParameters (AudioProcessorValueTreeState& parameters)
            : chorusMix (requireParameter (parameters, ParameterIds::MasterFX::chorusMix)),
              chorusRate (requireParameter (parameters, ParameterIds::MasterFX::chorusRate)),
              chorusDepth (requireParameter (parameters, ParameterIds::MasterFX::chorusDepth)),
              chorusDelay (requireParameter (parameters, ParameterIds::MasterFX::chorusDelay)),
              chorusFeedback (requireParameter (parameters, ParameterIds::MasterFX::chorusFeedback)),
              phaserMix (requireParameter (parameters, ParameterIds::MasterFX::phaserMix)),
              phaserRate (requireParameter (parameters, ParameterIds::MasterFX::phaserRate)),
              phaserDepth (requireParameter (parameters, ParameterIds::MasterFX::phaserDepth)),
              phaserFrequency (requireParameter (parameters, ParameterIds::MasterFX::phaserFrequency)),
              phaserFeedback (requireParameter (parameters, ParameterIds::MasterFX::phaserFeedback)),
              reverbMix (requireParameter (parameters, ParameterIds::MasterFX::reverbMix)),
              reverbRoomsize (requireParameter (parameters, ParameterIds::MasterFX::reverbRoomsize)),
              reverbDamping (requireParameter (parameters, ParameterIds::MasterFX::reverbDamping)),
              reverbWidth (requireParameter (parameters, ParameterIds::MasterFX::reverbWidth)),
              delayMix (requireParameter (parameters, ParameterIds::MasterFX::delayMix)),
              delayTime (requireParameter (parameters, ParameterIds::MasterFX::delayTime)),
              delayFeedback (requireParameter (parameters, ParameterIds::MasterFX::delayFeedback))
        {
        }

        std::atomic<float>& chorusMix;
        std::atomic<float>& chorusRate;
        std::atomic<float>& chorusDepth;
        std::atomic<float>& chorusDelay;
        std::atomic<float>& chorusFeedback;
        std::atomic<float>& phaserMix;
        std::atomic<float>& phaserRate;
        std::atomic<float>& phaserDepth;
        std::atomic<float>& phaserFrequency;
        std::atomic<float>& phaserFeedback;
        std::atomic<float>& reverbMix;
        std::atomic<float>& reverbRoomsize;
        std::atomic<float>& reverbDamping;
        std::atomic<float>& reverbWidth;
        std::atomic<float>& delayMix;
        std::atomic<float>& delayTime;
        std::atomic<float>& delayFeedback;
    };
} // namespace ParameterModels
