/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    PluginParameterLayout.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "PluginParameterLayout.h"
#include "fx/MasterEffectChain.h"
#include "synth/engine/SynthLayer.h"

AudioProcessorValueTreeState::ParameterLayout createPluginParameterLayout (int synthLayerCount)
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;

    for (int instanceNumber = 1; instanceNumber <= synthLayerCount; ++instanceNumber)
        SynthLayer::addParametersForInstance (parameters, instanceNumber);

    MasterEffectChain::addParameters (parameters);
    return { parameters.begin(), parameters.end() };
}
