/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    FilterEnvelopeGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "FilterEnvelopeGUI.h"

namespace
{
    constexpr float maxEnvelopeTimeMs = 10000.0f;
    constexpr float sustainMaximum = 1.0f;
    constexpr float filterReleaseMaximumMs = 2000.0f;
    constexpr float envelopeCutoffMinimumHz = 200.0f;
    constexpr float envelopeCutoffMaximumHz = 22050.0f;
    constexpr int verticalSliderWidth = 40;
    constexpr int verticalSliderHeight = 155;
    constexpr int rotarySliderWidth = 45;
    constexpr int rotarySliderHeight = 45;
    constexpr int titleY = 15;
    constexpr int titleHeight = 20;
    constexpr int frameX = 5;
    constexpr int frameY = 30;
    constexpr float frameCornerSize = 20.0f;
    constexpr float frameLineThickness = 2.0f;
    constexpr int labelY = 35;
    constexpr int sliderY = 50;
    constexpr unsigned int horizontalPadding = 20;
} // namespace

//==============================================================================
FilterEnvelopeGUI::FilterEnvelopeGUI (AudioProcessorValueTreeState& parameters, int synthInstance)
{
    const auto ids = ParameterIds::Synth (synthInstance);

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        maxEnvelopeTimeMs,
        verticalSliderWidth,
        verticalSliderHeight,
        ids.filterAttack,
        "A",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        maxEnvelopeTimeMs,
        verticalSliderWidth,
        verticalSliderHeight,
        ids.filterDecay,
        "D",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        sustainMaximum,
        verticalSliderWidth,
        verticalSliderHeight,
        ids.filterSustain,
        "S",
        "",
        "",
        2));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        filterReleaseMaximumMs,
        verticalSliderWidth,
        verticalSliderHeight,
        ids.filterRelease,
        "R",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        envelopeCutoffMinimumHz,
        envelopeCutoffMaximumHz,
        rotarySliderWidth,
        rotarySliderHeight,
        ids.envelopeCutoffLimit,
        "Limit",
        " Hz"));
}

void FilterEnvelopeGUI::paint (Graphics& g)
{
    drawTitle (g, Colours::white, "Filter Envelope", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        static_cast<float> (frameX),
        static_cast<float> (frameY),
        static_cast<float> (getWidth() - frameX),
        static_cast<float> (getLocalBounds().getHeight() - (frameY * 2)),
        frameCornerSize,
        frameLineThickness);
}

void FilterEnvelopeGUI::resized() { setHorizontalSliderLayout (labelY, sliderY, horizontalPadding, horizontalPadding); }
