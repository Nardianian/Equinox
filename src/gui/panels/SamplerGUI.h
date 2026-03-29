/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SamplerGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "SynthGUI.h"
#include "state/AudioSampleValueTreeState.h"

//==============================================================================
class SamplerGUI : public SynthGUI
{
public:
    SamplerGUI (AudioProcessorValueTreeState& state, AudioSampleValueTreeState& sampleState, int layerIndex);
    ~SamplerGUI() override = default;

    void paint (Graphics&) override;
    void resized() override;

private:
    Label sampleLabel;
    TextButton loadSampleButton { "Load Sample" };

    void browseForSampleFile();

    AudioSampleValueTreeState& audioSamples;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerGUI)
};
