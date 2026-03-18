/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "EquinoxLookAndFeel.h"
#include "MainMenu.h"
#include "MasterEffectsGUI.h"
#include "PluginProcessor.h"
#include "SynthesizerUnitGUI.h"
#include <JuceHeader.h>


//==============================================================================
/**
*/
class EquinoxAudioProcessorEditor : public AudioProcessorEditor
{
public:
    EquinoxAudioProcessorEditor (EquinoxAudioProcessor&);
    ~EquinoxAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    TabMenu synthLayerTabMenu;

    int synthInstanceNum;
    SynthesizerUnitGUI synthUnitGUI1, synthUnitGUI2, synthUnitGUI3;

    MainMenu mainMenu;

    MasterEffectsGUI masterEffects;

    MidiKeyboardComponent keyboardComponent;

    EquinoxLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxAudioProcessorEditor)
};
