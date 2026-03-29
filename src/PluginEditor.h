#pragma once

#include "gui/components/EquinoxLookAndFeel.h"
#include "gui/panels/MainMenu.h"
#include "gui/panels/MasterEffectsGUI.h"
#include "gui/panels/SynthesizerUnitGUI.h"
#include <JuceHeader.h>

class EquinoxAudioProcessor;

//==============================================================================
/** Top-level editor that composes the menu, synth-layer panels, master effects, and keyboard UI. */
class EquinoxAudioProcessorEditor : public AudioProcessorEditor
{
public:
    /** Creates the editor for the supplied processor instance. */
    EquinoxAudioProcessorEditor (EquinoxAudioProcessor&);
    /** Destroys the editor and owned UI components. */
    ~EquinoxAudioProcessorEditor() override;

    /** Paints the editor background and static framing graphics. */
    void paint (Graphics&) override;
    /** Lays out the editor child components after a resize. */
    void resized() override;

private:
    TabMenu synthLayerTabMenu;

    int synthInstanceNum;
    SynthesizerUnitGUI synthUnitGUI1, synthUnitGUI2, synthUnitGUI3;

    MainMenu mainMenu;

    MasterEffectsGUI masterEffects;

    MidiKeyboardComponent keyboardComponent;

    EquinoxLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxAudioProcessorEditor)
};
