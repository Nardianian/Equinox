#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <GraphicsBinaryData.h>

namespace
{
    constexpr int keyboardHeight = 80;
    constexpr int mainMenuHeight = 170;
    constexpr int synthLayerTabsY = 170;
    constexpr int synthLayerTabsHeight = 280;
    constexpr int masterEffectsY = 450;
    constexpr int masterEffectsHeight = 80;
    constexpr int editorBaseHeight = 530;
    constexpr int editorWidth = 1050;
    constexpr int synthLayerCount = 3;
    const auto sliderThumbColour = juce::Colour (30.0f, 124.0f, 105.0f);
} // namespace

//==============================================================================
EquinoxAudioProcessorEditor::EquinoxAudioProcessorEditor (EquinoxAudioProcessor& p)
    : AudioProcessorEditor (&p),
      synthLayerTabMenu (TabbedButtonBar::Orientation::TabsAtTop),
      synthUnitGUI1 (p.getParameters(), p.getAudioSampleState(), 1),
      synthUnitGUI2 (p.getParameters(), p.getAudioSampleState(), 2),
      synthUnitGUI3 (p.getParameters(), p.getAudioSampleState(), 3),
      mainMenu (p.getPresetManager()),
      masterEffects (p.getParameters()),
      keyboardComponent (p.midiKeyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    setLookAndFeel (&lookAndFeel);
    getLookAndFeel().setColour (juce::Slider::thumbColourId, sliderThumbColour);

    synthInstanceNum = synthLayerCount;

    synthLayerTabMenu.addTab ("A", Colours::transparentBlack, &synthUnitGUI1, false);
    synthLayerTabMenu.addTab ("B", Colours::transparentBlack, &synthUnitGUI2, false);
    synthLayerTabMenu.addTab ("C", Colours::transparentBlack, &synthUnitGUI3, false);
    synthLayerTabMenu.setOutlineStyle (TabMenu::OutlineStyle::none);

    synthLayerTabMenu.setCurrentTabIndex (0);

    addAndMakeVisible (&synthLayerTabMenu);

    setSize (editorWidth, editorBaseHeight + keyboardHeight + 1);

    addAndMakeVisible (&mainMenu);

    addAndMakeVisible (&masterEffects);

    keyboardComponent.clearKeyMappings();
    keyboardComponent.setBounds (0, editorBaseHeight + 1, getWidth(), keyboardHeight);
    addAndMakeVisible (&keyboardComponent);

    setBufferedToImage (true);
}

EquinoxAudioProcessorEditor::~EquinoxAudioProcessorEditor() { setLookAndFeel (nullptr); }

//==============================================================================
void EquinoxAudioProcessorEditor::paint (Graphics& g)
{
    Image background =
        ImageCache::getFromMemory (GraphicsResources::Background_png, GraphicsResources::Background_pngSize);
    g.drawImageAt (background, 0, 0);
}

void EquinoxAudioProcessorEditor::resized()
{
    Rectangle<int> area = getLocalBounds();

    mainMenu.setBounds (0, 0, getBounds().getWidth(), mainMenuHeight);

    synthLayerTabMenu.setBounds (0, synthLayerTabsY, area.getWidth(), synthLayerTabsHeight);
    //    synthUnitGUI1.setBounds(0, 170, area.getWidth(), 200);
    //    synthUnitGUI2.setBounds(0, 370, area.getWidth(), 200);
    //    synthUnitGUI3.setBounds(0, 570, area.getWidth(), 200);

    masterEffects.setBounds (0, masterEffectsY, area.getWidth(), masterEffectsHeight);
}
