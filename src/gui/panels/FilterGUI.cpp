/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    FilterGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "FilterGUI.h"

namespace
{
    constexpr float cutoffMinimumHz = 5.0f;
    constexpr float cutoffMaximumHz = 22050.0f;
    constexpr float driveMaximum = 100.0f;
    constexpr float resonanceMaximum = 100.0f;
    constexpr int cutoffSliderSize = 70;
    constexpr int driveSliderSize = 50;
    constexpr int resonanceSliderSize = 70;
    constexpr int titleY = 15;
    constexpr int titleHeight = 20;
    constexpr int frameX = 5;
    constexpr int frameY = 30;
    constexpr float frameCornerSize = 20.0f;
    constexpr float frameLineThickness = 2.0f;
    constexpr int menuWidth = 100;
    constexpr int menuHeight = 20;
    constexpr int menuY = 40;
    constexpr int sliderLabelY = 100;
    constexpr int sliderY = 110;
    constexpr unsigned int horizontalPadding = 25;
    constexpr int driveSliderYOffset = -30;
} // namespace

//==============================================================================
FilterGUI::FilterGUI (AudioProcessorValueTreeState& parameters, int synthInstance) : synthParameterIds (synthInstance)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        cutoffMinimumHz,
        cutoffMaximumHz,
        cutoffSliderSize,
        cutoffSliderSize,
        synthParameterIds.cutoff,
        "Cutoff",
        " Hz"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        driveMaximum,
        driveSliderSize,
        driveSliderSize,
        synthParameterIds.drive,
        "Drive",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        resonanceMaximum,
        resonanceSliderSize,
        resonanceSliderSize,
        synthParameterIds.resonance,
        "Resonance",
        " %"));

    filterMenu.addItem ("LPF12", 1);
    filterMenu.addItem ("HPF12", 2);
    filterMenu.addItem ("LPF24", 3);
    filterMenu.addItem ("HPF24", 4);
    filterMenu.setJustificationType (Justification::centred);
    addAndMakeVisible (&filterMenu);

    filterTypeAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (
        parameters, synthParameterIds.filterType, filterMenu);
}

void FilterGUI::paint (Graphics& g)
{
    drawTitle (g, Colours::white, "Filter", 0, titleY, getWidth(), titleHeight);
    drawRectangle (g,
        Colours::darkgrey,
        static_cast<float> (frameX),
        static_cast<float> (frameY),
        static_cast<float> (getWidth() - frameX),
        static_cast<float> (getLocalBounds().getHeight() - (frameY * 2)),
        frameCornerSize,
        frameLineThickness);
}

void FilterGUI::resized()
{
    filterMenu.setSize (menuWidth, menuHeight);
    filterMenu.setBounds (
        roundToInt ((static_cast<float> (getWidth()) * 0.5f) - (static_cast<float> (filterMenu.getWidth()) * 0.5f)),
        menuY,
        filterMenu.getWidth(),
        filterMenu.getHeight());

    setHorizontalSliderLayout (sliderLabelY, sliderY, horizontalPadding, horizontalPadding);

    EquinoxSlider& driveSlider = *getSlider (synthParameterIds.drive);
    driveSlider.setPosition (driveSlider.getX(), driveSliderYOffset);
}
