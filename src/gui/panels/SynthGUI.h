/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "gui/components/EquinoxButton.h"
#include "gui/components/EquinoxComponent.h"
#include "state/ParameterIds.h"

//==============================================================================
class SynthGUI : public EquinoxComponent
{
public:
    SynthGUI (AudioProcessorValueTreeState& parameters, int synthInstance);
    ~SynthGUI() override;

    void paint (Graphics&) override;
    void resized() override;

protected:
    AudioProcessorValueTreeState& parameters;
    int synthInstance = 1;
    ParameterIds::Synth synthParameterIds;

private:
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> onOffAttachment;

    EquinoxButton monoButton { "", true, EquinoxButton::Style::indicatorToggle };
    EquinoxButton onOffButton { "On/Off", true };
    Label monoLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGUI)
};
