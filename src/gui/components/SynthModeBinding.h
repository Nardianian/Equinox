/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthModeBinding.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "state/ParameterIds.h"
#include <JuceHeader.h>

/** Small adapter that binds a synth-mode parameter to the synth tab UI. */
class SynthModeBinding
{
public:
    /** Callback type used to notify the UI about mode changes. */
    using ModeChangedCallback = std::function<void (int)>;

    /** Creates a binding for one synth layer's mode parameter. */
    SynthModeBinding (AudioProcessorValueTreeState& parameters,
        int synthInstance,
        ModeChangedCallback modeChangedCallback);

    /** Returns the current mode value from the bound parameter. */
    int getCurrentMode() const;
    /** Updates the bound parameter from a UI mode selection. */
    void setCurrentMode (int mode);
    /** Immediately pushes the current parameter value through the callback. */
    void sendInitialUpdate();

private:
    /** Returns the synth-mode parameter or asserts if it is missing. */
    static RangedAudioParameter& requireSynthModeParameter (AudioProcessorValueTreeState& parameters,
        const String& parameterId);

    AudioProcessorValueTreeState& parameters;
    ParameterIds::Synth synthParameterIds;
    ParameterAttachment attachment;
};
