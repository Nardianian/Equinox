/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    DelayGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "DelayGUI.h"

namespace
{
    constexpr float percentageMaximum = 100.0f;
    constexpr float delayTimeMinimum = 1.0f;
    constexpr float delayTimeMaximum = 8.0f;
    constexpr int sliderWidth = 40;
    constexpr int sliderHeight = 40;
    constexpr int titleY = 5;
    constexpr int titleHeight = 20;
    constexpr int labelY = 25;
    constexpr int sliderY = 35;
} // namespace

//==============================================================================
DelayGUI::DelayGUI (AudioProcessorValueTreeState& parameters)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::delayMix,
        "Mix",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        delayTimeMinimum,
        delayTimeMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::delayTime,
        "Time",
        "",
        "1/"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::delayFeedback,
        "Feedback",
        " %"));
}

DelayGUI::~DelayGUI() {}

void DelayGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Delay", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g, Colours::darkgrey, 0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()));
}

void DelayGUI::resized() { setHorizontalSliderLayout (labelY, sliderY); }
