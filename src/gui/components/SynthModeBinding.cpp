/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthModeBinding.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "SynthModeBinding.h"
#include <stdexcept>

RangedAudioParameter& SynthModeBinding::requireSynthModeParameter (AudioProcessorValueTreeState& parameters,
    const String& parameterId)
{
    if (auto* parameter = parameters.getParameter (parameterId))
        return *parameter;

    jassertfalse;
    throw std::runtime_error ("Missing synth mode parameter");
}

SynthModeBinding::SynthModeBinding (AudioProcessorValueTreeState& parameterState,
    int synthInstance,
    ModeChangedCallback modeChangedCallback)
    : parameters (parameterState),
      synthParameterIds (synthInstance),
      attachment (
          requireSynthModeParameter (parameterState, synthParameterIds.synthMode),
          [callback = std::move (modeChangedCallback)] (float newValue) { callback (roundToInt (newValue)); },
          nullptr)
{
}

int SynthModeBinding::getCurrentMode() const
{
    if (auto* value = parameters.getRawParameterValue (synthParameterIds.synthMode))
        return roundToInt (value->load());

    jassertfalse;
    return 0;
}

void SynthModeBinding::setCurrentMode (int mode) { attachment.setValueAsCompleteGesture (static_cast<float> (mode)); }

void SynthModeBinding::sendInitialUpdate() { attachment.sendInitialUpdate(); }
