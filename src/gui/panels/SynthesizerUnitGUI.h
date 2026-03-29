/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    SynthesizerUnitGUI.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "AmpEnvelopeGUI.h"
#include "FilterEnvelopeGUI.h"
#include "FilterGUI.h"
#include "SynthMenuGUI.h"

//==============================================================================
/** Container for all controls that belong to a single synth layer. */
class SynthesizerUnitGUI : public Component,
                           private AudioProcessorValueTreeState::Listener,
                           private AsyncUpdater
{
public:
    /** Creates the complete synth-layer editor for one layer instance. */
    SynthesizerUnitGUI (AudioProcessorValueTreeState& parameters,
        AudioSampleValueTreeState& audioSamples,
        int instanceNum);
    ~SynthesizerUnitGUI() override;

    /** Paints the synth-unit section background. */
    void paint (Graphics&) override;
    /** Draws whole-layer state cues above the child panels. */
    void paintOverChildren (Graphics&) override;
    /** Lays out the child synth, filter, and envelope panels. */
    void resized() override;

private:
    [[nodiscard]] bool isLayerActive() const;
    void parameterChanged (const String& parameterID, float newValue) override;
    void handleAsyncUpdate() override;

    SynthMenuGUI synthMenuGUI;

    AmpEnvelopeGUI ampEnvelopeGUI;

    FilterEnvelopeGUI filterEnvelopeGUI;

    FilterGUI filterGUI;
    AudioProcessorValueTreeState& parameters;
    String activeParameterId;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesizerUnitGUI)
};
