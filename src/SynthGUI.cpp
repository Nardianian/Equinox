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

//==============================================================================
SynthGUI::SynthGUI (StateManager& stateManager, std::string synthInstance) : stateManager (stateManager), synthInstance (synthInstance)
{
    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "amplitude" + synthInstance, "Amp", "%"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, -24, 24, 40, 40, "pitchTranspose" + synthInstance, "Transpose"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, -100.0f, 100.0f, 40, 40, "finePitch" + synthInstance, "Fine Pitch", " Cent"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "analogValue" + synthInstance, "Analog Factor"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, -100.0f, 100.0f, 40, 40, "oscPanning" + synthInstance, "Pan"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, -20.0f, 20.0f, 40, 40, "detune" + synthInstance, "Detune"));

    addEquinoxSlider (std::make_shared<EquinoxSlider> (stateManager, Slider::SliderStyle::RotaryHorizontalVerticalDrag, 0.0f, 100.0f, 40, 40, "portamento" + synthInstance, "Portamento"));

    // On/Off button
    onOffAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (stateManager.getParameters(), "isActive" + synthInstance, onOffButton);
    addAndMakeVisible (onOffButton);

    // Mono button
    monoAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (stateManager.getParameters(), "monoEnabled" + synthInstance, monoButton);
    addAndMakeVisible (monoButton);
}

SynthGUI::~SynthGUI()
{
}

void SynthGUI::paint (Graphics& g)
{
}

void SynthGUI::resized()
{
    EquinoxSlider& amplitudeSlider = *getSlider ("amplitude" + synthInstance);
    amplitudeSlider.setBounds (155, 0, 50, 70);
    amplitudeSlider.setSliderComponentBounds (amplitudeSlider.getSliderComponentBounds().getX(), 14);
    amplitudeSlider.setLabelComponentBounds (amplitudeSlider.getLabelComponentBounds().getX(), 2, amplitudeSlider.getWidth(), 30);
    amplitudeSlider.center();

    EquinoxSlider& panningSlider = *getSlider ("oscPanning" + synthInstance);
    panningSlider.setBounds (155, 46, 50, 70);
    panningSlider.setSliderComponentBounds (panningSlider.getSliderComponentBounds().getX(), 14);
    panningSlider.setLabelComponentBounds (panningSlider.getLabelComponentBounds().getX(), 2, panningSlider.getWidth(), 30);
    panningSlider.center();

    EquinoxSlider& portamentoSlider = *getSlider ("portamento" + synthInstance);
    portamentoSlider.setBounds (50, 46, 50, 70);
    portamentoSlider.setSliderComponentBounds (portamentoSlider.getSliderComponentBounds().getX(), 14);
    portamentoSlider.setLabelComponentBounds (portamentoSlider.getLabelComponentBounds().getX(), 2, portamentoSlider.getWidth(), 30);
    portamentoSlider.center();

    EquinoxSlider& analogSlider = *getSlider ("analogValue" + synthInstance);
    analogSlider.setBounds (5, 93, 50, 70);
    analogSlider.setSliderComponentBounds (analogSlider.getSliderComponentBounds().getX(), 14);
    analogSlider.setLabelComponentBounds (analogSlider.getLabelComponentBounds().getX(), 2, analogSlider.getWidth(), 30);
    analogSlider.center();

    EquinoxSlider& finePitchSlider = *getSlider ("finePitch" + synthInstance);
    finePitchSlider.setBounds (50, 93, 50, 70);
    finePitchSlider.setSliderComponentBounds (finePitchSlider.getSliderComponentBounds().getX(), 14);
    finePitchSlider.setLabelComponentBounds (finePitchSlider.getLabelComponentBounds().getX(), 2, finePitchSlider.getWidth(), 30);
    finePitchSlider.center();

    EquinoxSlider& transposeSlider = *getSlider ("pitchTranspose" + synthInstance);
    transposeSlider.setBounds (105, 93, 50, 70);
    transposeSlider.setSliderComponentBounds (transposeSlider.getSliderComponentBounds().getX(), 14);
    transposeSlider.setLabelComponentBounds (transposeSlider.getLabelComponentBounds().getX(), 2, transposeSlider.getWidth(), 30);
    transposeSlider.center();

    EquinoxSlider& detuneSlider = *getSlider ("detune" + synthInstance);
    detuneSlider.setBounds (155, 93, 50, 70);
    detuneSlider.setSliderComponentBounds (detuneSlider.getSliderComponentBounds().getX(), 14);
    detuneSlider.setLabelComponentBounds (detuneSlider.getLabelComponentBounds().getX(), 2, detuneSlider.getWidth(), 30);
    detuneSlider.center();

    onOffButton.setBounds (110, 20, 40, 18);

    monoButton.setBounds (5, 71, 48, 18);
}
