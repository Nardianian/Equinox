/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SamplerGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "SamplerGUI.h"

namespace
{
    constexpr int leftInset = 6;
    constexpr int controlLabelY = 4;
    constexpr int controlLabelWidth = 120;
    constexpr int controlLabelHeight = 18;
    constexpr float controlLabelMinimumScale = 0.1f;
    constexpr int loadButtonX = leftInset;
    constexpr int loadButtonWidth = 120;
    constexpr int loadButtonHeight = 20;
    constexpr int topRowSliderCentreY = 4 + 14 + (46 / 2);
    constexpr int loadButtonY = topRowSliderCentreY - (loadButtonHeight / 2);
    constexpr float sampleStartMaximum = 100.0f;
    constexpr int sampleStartSliderSize = 46;
    constexpr int sampleStartSliderX = 128;
    constexpr int sampleStartSliderY = 62;
    constexpr int sliderBoundsWidth = 62;
    constexpr int sliderBoundsHeight = 76;
    constexpr int sliderComponentY = 14;
    constexpr int labelComponentY = 2;
    constexpr int labelComponentHeight = 30;
} // namespace

//==============================================================================
SamplerGUI::SamplerGUI (AudioProcessorValueTreeState& state, AudioSampleValueTreeState& sampleState, int layerIndex)
    : SynthGUI (state, layerIndex), audioSamples (sampleState)
{
    loadSampleButton.onClick = [&]() { browseForSampleFile(); };
    addAndMakeVisible (loadSampleButton);

    sampleLabel.setText ("Sample", NotificationType::dontSendNotification);
    sampleLabel.setMinimumHorizontalScale (controlLabelMinimumScale);
    sampleLabel.setJustificationType (Justification::centredLeft);
    sampleLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (sampleLabel);

    // sample start time slider
    addEquinoxSlider (std::make_unique<EquinoxSlider> (state,
        Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        0.0f,
        sampleStartMaximum,
        sampleStartSliderSize,
        sampleStartSliderSize,
        synthParameterIds.sampleStartTime,
        "Start time",
        " %",
        "",
        2));
}

void SamplerGUI::browseForSampleFile()
{
    FileChooser fileChooser ("Load audio sample", File(), "*.wav;*.mp3;*.flac", true, false, nullptr);

    if (fileChooser.browseForFileToOpen())
    {
        File sampleFile = fileChooser.getResult();

        audioSamples.addAudioSample (sampleFile, synthParameterIds.sampleStateId);
    }
}

void SamplerGUI::paint (Graphics& g) { SynthGUI::paint (g); }

void SamplerGUI::resized()
{
    sampleLabel.setBounds (loadButtonX, controlLabelY, controlLabelWidth, controlLabelHeight);
    loadSampleButton.setBounds (loadButtonX, loadButtonY, loadButtonWidth, loadButtonHeight);

    EquinoxSlider& sampleStartSlider = *getSlider (synthParameterIds.sampleStartTime);
    sampleStartSlider.setBounds (sampleStartSliderX, sampleStartSliderY, sliderBoundsWidth, sliderBoundsHeight);
    sampleStartSlider.setSliderComponentBounds (sampleStartSlider.getSliderComponentBounds().getX(), sliderComponentY);
    sampleStartSlider.setLabelComponentBounds (sampleStartSlider.getLabelComponentBounds().getX(),
        labelComponentY,
        sampleStartSlider.getWidth(),
        labelComponentHeight);
    sampleStartSlider.center();

    SynthGUI::resized();
}
