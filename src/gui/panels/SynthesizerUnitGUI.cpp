/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthesizerUnitGUI.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/
#include "SynthesizerUnitGUI.h"

namespace
{
    constexpr float inactiveLayerOverlayAlpha = 0.32f;
    constexpr float layerCornerSize = 6.0f;
    const auto inactiveLayerOverlayColour = juce::Colour::fromRGBA (6, 8, 10, 255);
}

//==============================================================================

SynthesizerUnitGUI::SynthesizerUnitGUI (AudioProcessorValueTreeState& parameterState,
    AudioSampleValueTreeState& audioSamples,
    int instanceNum)
    : synthMenuGUI (parameterState, audioSamples, instanceNum),
      ampEnvelopeGUI (parameterState, instanceNum),
      filterEnvelopeGUI (parameterState, instanceNum),
      filterGUI (parameterState, instanceNum),
      parameters (parameterState),
      activeParameterId (ParameterIds::Synth (instanceNum).isActive)
{
    parameters.addParameterListener (activeParameterId, this);
    addAndMakeVisible (&synthMenuGUI);
    addAndMakeVisible (&ampEnvelopeGUI);
    addAndMakeVisible (&filterEnvelopeGUI);
    addAndMakeVisible (&filterGUI);
}

SynthesizerUnitGUI::~SynthesizerUnitGUI() { parameters.removeParameterListener (activeParameterId, this); }

void SynthesizerUnitGUI::paint (Graphics& g) { ignoreUnused (g); }

void SynthesizerUnitGUI::paintOverChildren (Graphics& g)
{
    if (!isLayerActive())
    {
        g.setColour (inactiveLayerOverlayColour.withMultipliedAlpha (inactiveLayerOverlayAlpha));
        g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f, 1.0f), layerCornerSize);
    }
}

bool SynthesizerUnitGUI::isLayerActive() const
{
    if (auto* activeValue = parameters.getRawParameterValue (activeParameterId))
        return activeValue->load (std::memory_order_relaxed) >= 0.5f;

    return false;
}

void SynthesizerUnitGUI::parameterChanged (const String& parameterID, float newValue)
{
    ignoreUnused (newValue);

    if (parameterID == activeParameterId)
        triggerAsyncUpdate();
}

void SynthesizerUnitGUI::handleAsyncUpdate()
{
    repaint();
}

void SynthesizerUnitGUI::resized()
{
    Rectangle<int> area = getLocalBounds();

    const int componentCount = 4;

    const int componentWidth = area.getWidth() / componentCount;
    const int componentHeight = area.getHeight();

    synthMenuGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (componentHeight));

    ampEnvelopeGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (componentHeight));

    filterEnvelopeGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (componentHeight));

    filterGUI.setBounds (area.removeFromLeft (componentWidth).removeFromTop (componentHeight));
}
