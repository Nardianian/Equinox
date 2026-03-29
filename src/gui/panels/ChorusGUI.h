/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Chorus.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "gui/components/EquinoxComponent.h"
#include "state/ParameterIds.h"

//==============================================================================
class ChorusGUI : public EquinoxComponent
{
public:
    ChorusGUI (AudioProcessorValueTreeState& parameters);
    ~ChorusGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusGUI)
};
