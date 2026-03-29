/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthMenuGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "OscillatorGUI.h"
#include "SamplerGUI.h"
#include "gui/components/EquinoxComponent.h"
#include "gui/components/SynthModeBinding.h"
#include "gui/components/TabMenu.h"
#include "state/AudioSampleValueTreeState.h"

//==============================================================================
/** Panel that switches between the oscillator and sampler editors for one synth layer. */
class SynthMenuGUI : public EquinoxComponent
{
public:
    /** Creates the synth-mode tab menu and the child layer editors. */
    SynthMenuGUI (AudioProcessorValueTreeState& parameters, AudioSampleValueTreeState& audioSamples, int synthInstance);

    /** Destroys the tab menu and child editors. */
    ~SynthMenuGUI() override;

    /** Paints the synth-menu frame. */
    void paint (Graphics&) override;
    /** Lays out the tab menu and child editors. */
    void resized() override;

private:
    /** Synchronizes the selected tab back into the bound synth-mode parameter. */
    void tabSwitched();

    OscillatorGUI oscillatorGUI;

    SamplerGUI samplerGUI;

    TabMenu synthModeTabMenu;
    SynthModeBinding synthModeBinding;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthMenuGUI)
};
