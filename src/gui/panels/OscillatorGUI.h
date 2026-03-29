/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    OscillatorGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "SynthGUI.h"

//==============================================================================
class OscillatorGUI : public SynthGUI
{
public:
    OscillatorGUI (AudioProcessorValueTreeState& state, int layerIndex);
    ~OscillatorGUI() override = default;

    void paint (Graphics&) override;
    void resized() override;

private:
    Label waveformLabel;
    ComboBox waveformMenu;

    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> waveformMenuAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorGUI)
};
