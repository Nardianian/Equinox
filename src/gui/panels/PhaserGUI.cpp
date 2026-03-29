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

namespace
{
    constexpr float percentageMaximum = 100.0f;
    constexpr float phaserRateMaximumHz = 1.0f;
    constexpr float phaserFrequencyMinimumHz = 100.0f;
    constexpr float phaserFrequencyMaximumHz = 22000.0f;
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
PhaserGUI::PhaserGUI (AudioProcessorValueTreeState& parameters)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::phaserMix,
        "Mix",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        phaserRateMaximumHz,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::phaserRate,
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
        ParameterIds::MasterFX::phaserDepth,
        "Depth",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        phaserFrequencyMinimumHz,
        phaserFrequencyMaximumHz,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::phaserFrequency,
        "Freq",
        " Hz"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        -100.0f,
        percentageMaximum,
        sliderWidth,
        sliderHeight,
        ParameterIds::MasterFX::phaserFeedback,
        "Feedback",
        " %"));
}

PhaserGUI::~PhaserGUI() {}

void PhaserGUI::paint (juce::Graphics& g)
{
    drawTitle (g, Colours::white, "Phaser", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        0.0f,
        0.0f,
        static_cast<float> (getWidth()),
        static_cast<float> (getHeight()),
        frameCornerSize,
        frameLineThickness);
}

void PhaserGUI::resized() { setHorizontalSliderLayout (labelY, sliderY); }
