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

namespace
{
    constexpr float percentageMaximum = 100.0f;
    constexpr int sliderWidth = 40;
    constexpr int sliderHeight = 40;
    constexpr int titleY = 5;
    constexpr int titleHeight = 20;
    constexpr int labelY = 25;
    constexpr int sliderY = 35;
    constexpr float frameCornerSize = 1.0f;
    constexpr float frameLineThickness = 2.0f;
} // namespace

//==============================================================================
ReverbGUI::ReverbGUI (AudioProcessorValueTreeState& parameters)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::reverbMix,
        "Mix",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::reverbRoomsize,
        "Size",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::reverbDamping,
        "Damping",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::reverbWidth,
        "Width",
        " %"));
}

ReverbGUI::~ReverbGUI() {}

void ReverbGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Reverb", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        0.0f,
        0.0f,
        static_cast<float> (getWidth()),
        static_cast<float> (getHeight()),
        frameCornerSize,
        frameLineThickness);
}

void ReverbGUI::resized() { setHorizontalSliderLayout (labelY, sliderY); }
