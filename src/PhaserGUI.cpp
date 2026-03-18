/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    PhaserGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "PhaserGUI.h"
#include <JuceHeader.h>


//==============================================================================
PhaserGUI::PhaserGUI (StateManager& stateManager)
{
    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "phaserMix", "Mix", "%"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 1.0f, 40, 40, "phaserRate", "Rate", "Hz", "", 2));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "phaserDepth", "Depth", "%"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 100.0f, 22000.0f, 40, 40, "phaserFrequency", "Freq", "Hz"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, -100.0f, 100.0f, 40, 40, "phaserFeedback", "Feedback"));
}

PhaserGUI::~PhaserGUI()
{
}

void PhaserGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Phaser", 0, 5, getWidth(), 20);
    drawRectangle (g, Colours::darkgrey, 0, 0, getWidth(), getHeight(), 1.0f, 2.0f);
}

void PhaserGUI::resized()
{
    setHorizontalSliderLayout (25, 35);
}
