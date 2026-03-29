/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    OscillatorGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "OscillatorGUI.h"

namespace
{
    constexpr int leftInset = 6;
    constexpr int controlLabelY = 4;
    constexpr int controlLabelWidth = 120;
    constexpr int controlLabelHeight = 18;
    constexpr float controlLabelMinimumScale = 0.1f;
    constexpr int waveformMenuWidth = 120;
    constexpr int waveformMenuHeight = 20;
    constexpr int waveformMenuX = leftInset;
    constexpr int topRowSliderCentreY = 4 + 14 + (46 / 2);
    constexpr int waveformMenuY = topRowSliderCentreY - (waveformMenuHeight / 2);
    constexpr int phaseSliderX = 128;
    constexpr int phaseSliderY = 62;
    constexpr int rotarySliderBoundsWidth = 62;
    constexpr int rotarySliderBoundsHeight = 76;
    constexpr int sliderComponentY = 14;
    constexpr int labelComponentY = 2;
    constexpr int labelComponentHeight = 30;
    constexpr float phaseMaximum = 100.0f;
    constexpr int phaseSliderSize = 46;
} // namespace

//==============================================================================
OscillatorGUI::OscillatorGUI (AudioProcessorValueTreeState& state, int layerIndex) : SynthGUI (state, layerIndex)
{
    // Waveform menu
    waveformMenu.addItem ("Sine", 1);
    waveformMenu.addItem ("Saw", 2);
    waveformMenu.addItem ("Triangle", 3);
    waveformMenu.addItem ("Square", 4);
    waveformMenu.addItem ("Noise", 5);
    waveformMenu.setJustificationType (Justification::centred);
    addAndMakeVisible (&waveformMenu);

    waveformMenuAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (
        state, synthParameterIds.waveform, waveformMenu);

    waveformLabel.setText ("Waveform", NotificationType::dontSendNotification);
    waveformLabel.setMinimumHorizontalScale (controlLabelMinimumScale);
    waveformLabel.setJustificationType (Justification::centredLeft);
    waveformLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (waveformLabel);

    // Phase slider
    addEquinoxSlider (std::make_unique<EquinoxSlider> (state,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        phaseMaximum,
        phaseSliderSize,
        phaseSliderSize,
        synthParameterIds.phase,
        "Phase",
        " %",
        "",
        2));
}

void OscillatorGUI::paint (Graphics& g) { SynthGUI::paint (g); }

void OscillatorGUI::resized()
{
    waveformLabel.setBounds (waveformMenuX, controlLabelY, controlLabelWidth, controlLabelHeight);
    waveformMenu.setBounds (waveformMenuX, waveformMenuY, waveformMenuWidth, waveformMenuHeight);

    EquinoxSlider& phaseSlider = *getSlider (synthParameterIds.phase);
    phaseSlider.setBounds (phaseSliderX, phaseSliderY, rotarySliderBoundsWidth, rotarySliderBoundsHeight);
    phaseSlider.setSliderComponentBounds (phaseSlider.getSliderComponentBounds().getX(), sliderComponentY);
    phaseSlider.setLabelComponentBounds (
        phaseSlider.getLabelComponentBounds().getX(), labelComponentY, phaseSlider.getWidth(), labelComponentHeight);
    phaseSlider.center();

    SynthGUI::resized();
}
