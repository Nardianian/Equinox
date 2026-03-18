/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    ReverbGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "ReverbGUI.h"
#include <JuceHeader.h>


//==============================================================================
ReverbGUI::ReverbGUI (StateManager& stateManager)
{
    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "reverbMix", "Mix", "%"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "reverbRoomsize", "Size"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "reverbDamping", "Damping"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "reverbWidth", "Width"));
}

ReverbGUI::~ReverbGUI()
{
}

void ReverbGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Reverb", 0, 5, getWidth(), 20);
    drawRectangle (g, Colours::darkgrey, 0, 0, getWidth(), getHeight(), 1.0f, 2.0f);
}

void ReverbGUI::resized()
{
    setHorizontalSliderLayout (25, 35);
}
