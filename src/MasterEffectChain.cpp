/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    MasterEffectChain.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "MasterEffectChain.h"

MasterEffectChain::MasterEffectChain (StateManager& state) : state (state)
{
}

void MasterEffectChain::initialize()
{
    state.addListener (this);
    updateParameters();
}

MasterEffectChain::~MasterEffectChain()
{
}

void MasterEffectChain::addParameters (std::vector<std::unique_ptr<RangedAudioParameter>>& params)
{
    // Chorus parameters
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("chorusMix", 1), "Chorus mix", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("chorusRate", 1), "Chorus rate", NormalisableRange<float> (0.0f, 5.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("chorusDepth", 1), "Chorus depth", NormalisableRange<float> (0.0f, 0.15f, 0.0015f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("chorusDelay", 1), "Chorus delay", NormalisableRange<float> (1.0f, 30.0f, 0.29f, 1.0f), 1.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("chorusFeedback", 1), "Chorus feedback", NormalisableRange<float> (-1.0f, 1.0f, 0.02f, 1.0f), 0.0f));

    // Phaser parameters
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("phaserMix", 1), "Phaser mix", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("phaserRate", 1), "Phaser rate", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("phaserDepth", 1), "Phaser depth", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("phaserFrequency", 1), "Phaser frequency", NormalisableRange<float> (100.0f, 22000.0f, 2.0f, 1.0f), 11000.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("phaserFeedback", 1), "Phaser feedback", NormalisableRange<float> (-0.9f, 0.9f, 0.018f, 1.0f), 0.0f));

    // Reverb parameters
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("reverbMix", 1), "Reverb mix", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("reverbRoomsize", 1), "Reverb roomsize", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("reverbDamping", 1), "Reverb damping", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("reverbWidth", 1), "Reverb width", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    // Delay parameters
    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("delayMix", 1), "Delay mix", NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("delayTime", 1), "Delay time", NormalisableRange<float> (1.0f, 8.0f, 1.0f, 1.0f), 1.0f));

    params.push_back (std::make_unique<AudioParameterFloat> (ParameterID ("delayFeedback", 1), "Delay feedback", NormalisableRange<float> (0.0f, 100.0f, 1.0f, 1.0f), 0.0f));
}

void MasterEffectChain::prepareToPlay (int samplerate, int samplesPerBlock, int numChannels)
{
    reset();
    spec.sampleRate = samplerate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;

    chorusFX.prepare (spec);

    phaserFX.prepare (spec);

    reverbFX.prepare (spec);

    delayFX.prepare (spec);
}

void MasterEffectChain::reset()
{
    chorusFX.reset();

    phaserFX.reset();

    reverbFX.reset();

    delayFX.reset();
}

void MasterEffectChain::updateParameters()
{
    chorusFX.setParameters (*state.getAudioParameterValue ("chorusRate"),
        *state.getAudioParameterValue ("chorusDepth"),
        *state.getAudioParameterValue ("chorusDelay"),
        *state.getAudioParameterValue ("chorusFeedback"),
        *state.getAudioParameterValue ("chorusMix"));

    phaserFX.setParameters (*state.getAudioParameterValue ("phaserRate"),
        *state.getAudioParameterValue ("phaserDepth"),
        *state.getAudioParameterValue ("phaserFrequency"),
        *state.getAudioParameterValue ("phaserFeedback"),
        *state.getAudioParameterValue ("phaserMix"));

    reverbFX.setParameters (*state.getAudioParameterValue ("reverbRoomsize"),
        *state.getAudioParameterValue ("reverbDamping"),
        *state.getAudioParameterValue ("reverbWidth"),
        *state.getAudioParameterValue ("reverbMix"));

    delayFX.setParameters (*state.getAudioParameterValue ("delayTime"),
        *state.getAudioParameterValue ("delayFeedback"),
        *state.getAudioParameterValue ("delayMix"));
}

void MasterEffectChain::process (AudioBuffer<float>& bufferToProcess, double& bpm)
{
    if (needsUpdate)
    {
        updateParameters();
        needsUpdate = false;
    }

    chorusFX.process (bufferToProcess);
    phaserFX.process (bufferToProcess);
    reverbFX.process (bufferToProcess);
    delayFX.process (bufferToProcess, bpm);
}

void MasterEffectChain::valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
{
    // Updating the effects parameter values if the added child is the params
    if (childWhichHasBeenAdded.getType() == state.getParameters().state.getType())
    {
        needsUpdate = true;
    }
}

void MasterEffectChain::valueTreePropertyChanged (ValueTree& valueTree, const Identifier& propertyId)
{
    // Updating the effects parameter values if the valuetree which property has changed is a parameter
    if (valueTree.getType().toString() == "PARAM")
    {
        needsUpdate = true;
    }
}
