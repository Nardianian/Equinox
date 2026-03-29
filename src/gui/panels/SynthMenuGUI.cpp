/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthMenuGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "SynthMenuGUI.h"

namespace
{
    constexpr int tabMenuBottomPadding = 30;
    constexpr float roundedOutlineCornerSize = 20.0f;
    constexpr float roundedOutlineThickness = 2.0f;
} // namespace

//==============================================================================
SynthMenuGUI::SynthMenuGUI (AudioProcessorValueTreeState& parameterState,
    AudioSampleValueTreeState& audioSamples,
    int synthInstance)
    : oscillatorGUI (parameterState, synthInstance),
      samplerGUI (parameterState, audioSamples, synthInstance),
      synthModeTabMenu (TabbedButtonBar::Orientation::TabsAtTop),
      synthModeBinding (parameterState, synthInstance, [this] (int currentMode) {
          if (synthModeTabMenu.getCurrentTabIndex() != currentMode)
              synthModeTabMenu.setCurrentTabIndex (currentMode);
      })
{
    synthModeTabMenu.addTab ("Oscillator", Colours::transparentBlack, &oscillatorGUI, false);
    synthModeTabMenu.addTab ("Sampler", Colours::transparentBlack, &samplerGUI, false);
    synthModeTabMenu.setOutlineStyle (TabMenu::OutlineStyle::rounded);
    synthModeTabMenu.setRoundedOutlineAppearance (Colours::darkgrey, roundedOutlineCornerSize, roundedOutlineThickness);
    synthModeBinding.sendInitialUpdate();
    synthModeTabMenu.setCurrentTabIndex (synthModeBinding.getCurrentMode());

    synthModeTabMenu.onTabChanged = [&]() { tabSwitched(); };
    addAndMakeVisible (&synthModeTabMenu);
}

SynthMenuGUI::~SynthMenuGUI() {}

// Called if the current tab gets switch to another tab
void SynthMenuGUI::tabSwitched()
{
    if (synthModeBinding.getCurrentMode() != synthModeTabMenu.getCurrentTabIndex())
        synthModeBinding.setCurrentMode (synthModeTabMenu.getCurrentTabIndex());
}

void SynthMenuGUI::paint (Graphics& g) { ignoreUnused (g); }

void SynthMenuGUI::resized()
{
    const auto contentWidth = getLocalBounds().getWidth();
    const auto contentHeight = getLocalBounds().getHeight() - tabMenuBottomPadding;
    synthModeTabMenu.setBounds (0, 0, contentWidth, contentHeight);
    oscillatorGUI.setSize (contentWidth, contentHeight);
    samplerGUI.setSize (contentWidth, contentHeight);
}
