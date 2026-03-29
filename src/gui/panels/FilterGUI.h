/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    FilterGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "gui/components/EquinoxComponent.h"
#include "state/ParameterIds.h"

//==============================================================================
class FilterGUI : public EquinoxComponent
{
public:
    FilterGUI (AudioProcessorValueTreeState& parameters, int synthInstance);
    ~FilterGUI() override = default;

    void paint (Graphics&) override;
    void resized() override;

private:
    ParameterIds::Synth synthParameterIds;
    ComboBox filterMenu;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterGUI)
};
