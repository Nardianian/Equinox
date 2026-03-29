/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    MainMenu.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "gui/components/EquinoxButton.h"
#include "state/PresetManager.h"

//==============================================================================
class MainMenu : public juce::Component, Value::Listener
{
public:
    MainMenu (PresetManager& presetManagerRef);
    ~MainMenu() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int lcdCharacterLimit = PresetManager::maxPresetNameLength;

    void updatePresetDisplayText();
    void drawLcdText (juce::Graphics& g,
        const juce::String& text,
        juce::Rectangle<float> area,
        const juce::Font& font) const;
    [[nodiscard]] juce::String fitTextToLcdCells (juce::String text, int maxCharacters) const;

    void displayOverwritePopUp();

    void valueChanged (Value& value) override;

    Font menuFont { FontOptions {} };
    juce::Rectangle<int> menuTitleBounds;
    juce::Rectangle<int> presetNameBounds;
    juce::String presetDisplayText;
    float lcdCharacterCellWidth = 12.0f;

    Value presetName;

    Value presetHasBeenEdited;

    ArrowButton backArrow;
    ArrowButton forwardArrow;

    EquinoxButton initializeButton { "Initialize" };
    EquinoxButton loadButton { "Load Preset" };
    EquinoxButton overwriteButton { "Overwrite" };
    EquinoxButton saveButton { "Save Preset" };

    PopupMenu overwritePopUp;

    Image menuScreen;
    Image screenReflection;
    float screenXPosition = 0.0f;

    PresetManager& presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainMenu)
};
