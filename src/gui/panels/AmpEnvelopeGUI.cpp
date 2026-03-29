/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AmpEnvelopeGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "AmpEnvelopeGUI.h"

namespace
{
    constexpr float maxEnvelopeTimeMs = 10000.0f;
    constexpr float sustainMaximum = 1.0f;
    constexpr int sliderWidth = 40;
    constexpr int sliderHeight = 155;
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
AmpEnvelopeGUI::AmpEnvelopeGUI (AudioProcessorValueTreeState& parameters, int synthInstance)
{
    const auto ids = ParameterIds::Synth (synthInstance);

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        maxEnvelopeTimeMs,
        sliderWidth,
        sliderHeight,
        ids.ampAttack,
        "A",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        maxEnvelopeTimeMs,
        sliderWidth,
        sliderHeight,
        ids.ampDecay,
        "D",
        " ms"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        sustainMaximum,
        sliderWidth,
        sliderHeight,
        ids.ampSustain,
        "S",
        "",
        "",
        2));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::LinearVertical,
        0.0f,
        maxEnvelopeTimeMs,
        sliderWidth,
        sliderHeight,
        ids.ampRelease,
        "R",
        " ms"));
}

void AmpEnvelopeGUI::paint (Graphics& g)
{
    drawTitle (g, Colours::white, "Amp Envelope", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        static_cast<float> (frameX),
        static_cast<float> (frameY),
        static_cast<float> (getWidth() - frameX),
        static_cast<float> (getLocalBounds().getHeight() - (frameY * 2)),
        frameCornerSize,
        frameLineThickness);
}

void AmpEnvelopeGUI::resized() { setHorizontalSliderLayout (labelY, sliderY, horizontalPadding, horizontalPadding); }
