/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    Chorus.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "ChorusGUI.h"

namespace
{
    constexpr float percentageMaximum = 100.0f;
    constexpr float chorusRateMaximumHz = 5.0f;
    constexpr float chorusDelayMaximumMs = 30.0f;
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
ChorusGUI::ChorusGUI (AudioProcessorValueTreeState& parameters)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::chorusMix,
        "Mix",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        chorusRateMaximumHz,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::chorusRate,
        "Rate",
        " Hz",
        "",
        2));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::chorusDepth,
        "Depth",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        chorusDelayMaximumMs,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::chorusDelay,
        "Delay",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        -100.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::chorusFeedback,
        "Feedback",
        " %"));
}

ChorusGUI::~ChorusGUI() {}

void ChorusGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Chorus", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        0.0f,
        0.0f,
        static_cast<float> (getWidth()),
        static_cast<float> (getHeight()),
        frameCornerSize,
        frameLineThickness);
}

void ChorusGUI::resized() { setHorizontalSliderLayout (labelY, sliderY); }
