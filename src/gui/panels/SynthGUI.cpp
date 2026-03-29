/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "SynthGUI.h"

namespace
{
    constexpr float percentageMaximum = 100.0f;
    constexpr float amplitudeMinimumDecibels = -100.0f;
    constexpr float amplitudeMaximumDecibels = 0.0f;
    constexpr float portamentoMaximumMilliseconds = 500.0f;
    constexpr int smallSliderWidth = 46;
    constexpr int smallSliderHeight = 46;
    constexpr int sliderBoundsWidth = 62;
    constexpr int sliderBoundsHeight = 76;
    constexpr int upperRowY = 4;
    constexpr int middleRowY = 62;
    constexpr int lowerRowY = 122;
    constexpr int sliderComponentY = 14;
    constexpr int labelComponentY = 2;
    constexpr int labelComponentHeight = 30;
    constexpr int topRowSliderCentreY = upperRowY + sliderComponentY + (smallSliderHeight / 2);
    constexpr int amplitudeX = 192;
    constexpr int portamentoX = 64;
    constexpr int analogX = 0;
    constexpr int finePitchX = 64;
    constexpr int transposeX = 128;
    constexpr int detuneX = 192;
    constexpr int onOffButtonX = 134;
    constexpr int onOffButtonWidth = 40;
    constexpr int onOffButtonHeight = 18;
    constexpr int onOffButtonY = topRowSliderCentreY - (onOffButtonHeight / 2);
    constexpr int monoControlX = 0;
    constexpr int monoControlY = middleRowY;
    constexpr int monoControlWidth = sliderBoundsWidth;
    constexpr int monoLabelY = 2;
    constexpr int monoLabelHeight = 30;
    constexpr int monoButtonWidth = 20;
    constexpr int monoButtonHeight = 20;
    constexpr int monoButtonY = sliderComponentY + ((smallSliderHeight - monoButtonHeight) / 2);
    constexpr int pitchTransposeMinimum = -24;
    constexpr int pitchTransposeMaximum = 24;

    void layoutRotarySlider (EquinoxSlider& slider, int x, int y)
    {
        slider.setBounds (x, y, sliderBoundsWidth, sliderBoundsHeight);
        slider.setSliderComponentBounds (slider.getSliderComponentBounds().getX(), sliderComponentY);
        slider.setLabelComponentBounds (
            slider.getLabelComponentBounds().getX(), labelComponentY, slider.getWidth(), labelComponentHeight);
        slider.center();
    }

    String formatAmplitudeText (float rawAmplitude)
    {
        if (rawAmplitude <= 0.0f)
            return "-inf dB";

        const auto amplitudeInDecibels = Decibels::gainToDecibels (rawAmplitude, amplitudeMinimumDecibels);
        return String (amplitudeInDecibels, 1, false) + " dB";
    }

    float parseAmplitudeText (const String& text)
    {
        const auto trimmedText = text.trim().toLowerCase();

        if (trimmedText.contains ("-inf"))
            return 0.0f;

        const auto amplitudeInDecibels =
            jlimit (amplitudeMinimumDecibels, amplitudeMaximumDecibels, trimmedText.getFloatValue());
        return Decibels::decibelsToGain (amplitudeInDecibels, amplitudeMinimumDecibels);
    }
} // namespace

//==============================================================================
SynthGUI::SynthGUI (AudioProcessorValueTreeState& parameterState, int synthInstanceNumber)
    : parameters (parameterState), synthInstance (synthInstanceNumber), synthParameterIds (synthInstanceNumber)
{
    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        amplitudeMinimumDecibels,
        amplitudeMaximumDecibels,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.amplitude,
        "Gain",
        "",
        "",
        1,
        formatAmplitudeText,
        parseAmplitudeText));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        static_cast<float> (pitchTransposeMinimum),
        static_cast<float> (pitchTransposeMaximum),
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.pitchTranspose,
        "Transpose",
        " st"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        -100.0f,
        percentageMaximum,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.finePitch,
        "Fine Pitch",
        " ct"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        percentageMaximum,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.analogValue,
        "Analog Drift",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        -100.0f,
        percentageMaximum,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.oscPanning,
        "Pan",
        " %"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        -20.0f,
        20.0f,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.detune,
        "Detune",
        " ct"));

    addEquinoxSlider (std::make_unique<EquinoxSlider> (parameters,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        portamentoMaximumMilliseconds,
        smallSliderWidth,
        smallSliderHeight,
        synthParameterIds.portamento,
        "Glide",
        " ms"));

    // On/Off button
    onOffAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (
        parameters, synthParameterIds.isActive, onOffButton);
    addAndMakeVisible (onOffButton);

    // Mono button
    monoAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (
        parameters, synthParameterIds.monoEnabled, monoButton);
    addAndMakeVisible (monoButton);

    monoLabel.setText ("Mono", NotificationType::dontSendNotification);
    monoLabel.setMinimumHorizontalScale (0.1f);
    monoLabel.setJustificationType (Justification::centredTop);
    monoLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (monoLabel);
}

void SynthGUI::paint (Graphics& g) { ignoreUnused (g); }

SynthGUI::~SynthGUI() {}

void SynthGUI::resized()
{
    EquinoxSlider& amplitudeSlider = *getSlider (synthParameterIds.amplitude);
    layoutRotarySlider (amplitudeSlider, amplitudeX, upperRowY);

    EquinoxSlider& panningSlider = *getSlider (synthParameterIds.oscPanning);
    layoutRotarySlider (panningSlider, amplitudeX, middleRowY);

    EquinoxSlider& portamentoSlider = *getSlider (synthParameterIds.portamento);
    layoutRotarySlider (portamentoSlider, portamentoX, middleRowY);

    EquinoxSlider& analogSlider = *getSlider (synthParameterIds.analogValue);
    layoutRotarySlider (analogSlider, analogX, lowerRowY);

    EquinoxSlider& finePitchSlider = *getSlider (synthParameterIds.finePitch);
    layoutRotarySlider (finePitchSlider, finePitchX, lowerRowY);

    EquinoxSlider& transposeSlider = *getSlider (synthParameterIds.pitchTranspose);
    layoutRotarySlider (transposeSlider, transposeX, lowerRowY);

    EquinoxSlider& detuneSlider = *getSlider (synthParameterIds.detune);
    layoutRotarySlider (detuneSlider, detuneX, lowerRowY);

    onOffButton.setBounds (onOffButtonX, onOffButtonY, onOffButtonWidth, onOffButtonHeight);

    monoLabel.setBounds (monoControlX, monoControlY + monoLabelY, monoControlWidth, monoLabelHeight);
    monoButton.setBounds (monoControlX + ((monoControlWidth - monoButtonWidth) / 2),
        monoControlY + monoButtonY,
        monoButtonWidth,
        monoButtonHeight);
}
