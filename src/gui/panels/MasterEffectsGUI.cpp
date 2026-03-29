/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    MasterEffectsGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "MasterEffectsGUI.h"

namespace
{
    constexpr int titleX = 5;
    constexpr int titleY = 5;
    constexpr int titleWidth = 67;
    constexpr int titleHeight = 22;
    constexpr int masterEffectsComponentCount = 4;
    constexpr int masterEffectsHeight = 80;
    constexpr float frameCornerSize = 1.0f;
    constexpr float frameLineThickness = 2.0f;
} // namespace

//==============================================================================
MasterEffectsGUI::MasterEffectsGUI (AudioProcessorValueTreeState& parameters)
    : chorusGUI (parameters), phaserGUI (parameters), reverbGUI (parameters), delayGUI (parameters)
{
    addAndMakeVisible (&chorusGUI);
    addAndMakeVisible (&phaserGUI);
    addAndMakeVisible (&reverbGUI);
    addAndMakeVisible (&delayGUI);
}

MasterEffectsGUI::~MasterEffectsGUI() {}

void MasterEffectsGUI::paint (juce::Graphics& g)
{
    Rectangle<int> titleArea (titleX, titleY, titleWidth, titleHeight);

    g.setColour (Colours::white);
    g.drawText ("Master FX", titleArea, Justification::topLeft);

    Rectangle<float> area (
        0.0f, 0.0f, static_cast<float> (titleArea.getWidth()), static_cast<float> (titleArea.getHeight()));

    g.setColour (Colours::darkgrey);
    g.drawRoundedRectangle (area, frameCornerSize, frameLineThickness);
}

void MasterEffectsGUI::resized()
{
    Rectangle<int> area = getLocalBounds();

    const int componentWidth = area.getWidth() / masterEffectsComponentCount;

    chorusGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (masterEffectsHeight));

    phaserGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (masterEffectsHeight));

    reverbGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (masterEffectsHeight));

    delayGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (masterEffectsHeight));
}
