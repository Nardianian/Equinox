/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    MainMenu.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "MainMenu.h"
#include <FontBinaryData.h>
#include <GraphicsBinaryData.h>

namespace
{
    constexpr float menuFontHeight = 14.5f;
    constexpr float menuFontHorizontalScale = 0.84f;
    constexpr float menuFontKerning = 0.0f;
    constexpr float backArrowPosition = 0.5f;
    constexpr float forwardArrowPosition = 0.0f;
    constexpr int screenY = 80;
    constexpr int screenLeftTextPadding = 52;
    constexpr int textRightPadding = 26;
    constexpr int titleY = 98;
    constexpr int titleWidthCells = 9;
    constexpr int titleHeight = 18;
    constexpr int presetNameY = 120;
    constexpr int presetNameHeight = 22;
    constexpr int backArrowX = 635;
    constexpr int forwardArrowX = 655;
    constexpr int arrowY = 103;
    constexpr int arrowSize = 12;
    constexpr int utilityButtonX = 715;
    constexpr int utilityButtonWidth = 60;
    constexpr int utilityButtonHeight = 16;
    constexpr int initializeButtonY = 80;
    constexpr int overwriteButtonY = 100;
    constexpr int saveButtonY = 120;
    constexpr int loadButtonY = 140;
    constexpr float minimumCellWidth = 6.0f;
    constexpr float textBaselineInset = 1.0f;
    const auto lcdTextColour = juce::Colour::fromRGB (10, 16, 8);

    Font createMenuFont()
    {
        if (auto typeface = Typeface::createSystemTypefaceFor (
                FontResources::_8bit_fortress_ttf, FontResources::_8bit_fortress_ttfSize))
        {
            return Font (FontOptions (typeface)
                    .withHeight (menuFontHeight)
                    .withHorizontalScale (menuFontHorizontalScale)
                    .withKerningFactor (menuFontKerning));
        }

        return Font (FontOptions ("Monaco", "Regular", menuFontHeight).withKerningFactor (menuFontKerning));
    }
}

//==============================================================================
MainMenu::MainMenu (PresetManager& presetManagerRef)
    : backArrow ("back", backArrowPosition, Colours::black),
      forwardArrow ("forward", forwardArrowPosition, Colours::black),
      presetManager (presetManagerRef)
{
    presetName = Value (*presetManager.getCurrentPresetName());
    presetName.addListener (this);

    presetHasBeenEdited = Value (*presetManager.getCurrentPresetHasBeenEdited());
    presetHasBeenEdited.addListener (this);

    menuScreen = ImageCache::getFromMemory (GraphicsResources::MenuScreen_png, GraphicsResources::MenuScreen_pngSize);
    screenReflection = ImageCache::getFromMemory (
        GraphicsResources::MenuScreenReflection_png, GraphicsResources::MenuScreenReflection_pngSize);

    menuFont = createMenuFont();
    updatePresetDisplayText();

    backArrow.onClick = [&]() { presetManager.getPreviousPreset(); };
    addAndMakeVisible (&backArrow);

    forwardArrow.onClick = [&]() { presetManager.getNextPreset(); };
    addAndMakeVisible (&forwardArrow);

    initializeButton.onClick = [&]() { presetManager.initializePreset(); };
    addAndMakeVisible (&initializeButton);

    loadButton.onClick = [&]() { presetManager.loadPreset(); };
    addAndMakeVisible (&loadButton);

    overwriteButton.onClick = [&]() { displayOverwritePopUp(); };
    addAndMakeVisible (&overwriteButton);

    saveButton.onClick = [&]() { presetManager.savePreset(); };
    addAndMakeVisible (&saveButton);

    overwritePopUp.addSectionHeader ("Overwrite preset?");
    overwritePopUp.addItem (1, "Confirm");
    overwritePopUp.addItem (2, "Cancel");
}

MainMenu::~MainMenu()
{
    presetName.removeListener (this);
    presetHasBeenEdited.removeListener (this);
}

void MainMenu::displayOverwritePopUp()
{
    if (presetManager.currentPresetExistsAsFile() && (bool) presetHasBeenEdited.getValue())
    {
        int result = overwritePopUp.show();

        if (result == 1)
        {
            // User confirmed to overwrite preset
            presetManager.overwritePreset();
        }
    }
}

void MainMenu::valueChanged (Value& value)
{
    if (presetName == value)
    {
        updatePresetDisplayText();
        repaint (presetNameBounds);
    }

    if (presetHasBeenEdited == value)
    {
        updatePresetDisplayText();
        repaint (presetNameBounds);
    }
}

void MainMenu::paint (juce::Graphics& g)
{
    g.drawImageAt (menuScreen, roundToInt (screenXPosition), screenY);
    drawLcdText (g, "PRESET", menuTitleBounds.toFloat(), menuFont.withHeight (menuFontHeight));
    drawLcdText (g, presetDisplayText, presetNameBounds.toFloat(), menuFont.withHeight (menuFontHeight));
    g.drawImageAt (screenReflection, roundToInt (screenXPosition), screenY);
}

void MainMenu::resized()
{
    screenXPosition = static_cast<float> ((getLocalBounds().getWidth() - menuScreen.getWidth()) / 2);
    const auto screenLeft = roundToInt (screenXPosition);
    const auto textLeft = screenLeft + screenLeftTextPadding;
    const auto textRight = backArrowX - textRightPadding;
    const auto availableWidth = textRight - textLeft;

    lcdCharacterCellWidth = std::floor (static_cast<float> (availableWidth) / static_cast<float> (lcdCharacterLimit));
    menuTitleBounds = {
        textLeft, titleY, roundToInt (lcdCharacterCellWidth * static_cast<float> (titleWidthCells)), titleHeight
    };
    presetNameBounds = { textLeft,
        presetNameY,
        roundToInt (lcdCharacterCellWidth * static_cast<float> (lcdCharacterLimit)),
        presetNameHeight };

    backArrow.setBounds (backArrowX, arrowY, arrowSize, arrowSize);
    forwardArrow.setBounds (forwardArrowX, arrowY, arrowSize, arrowSize);
    initializeButton.setBounds (utilityButtonX, initializeButtonY, utilityButtonWidth, utilityButtonHeight);
    overwriteButton.setBounds (utilityButtonX, overwriteButtonY, utilityButtonWidth, utilityButtonHeight);
    saveButton.setBounds (utilityButtonX, saveButtonY, utilityButtonWidth, utilityButtonHeight);
    loadButton.setBounds (utilityButtonX, loadButtonY, utilityButtonWidth, utilityButtonHeight);
}

void MainMenu::updatePresetDisplayText()
{
    presetDisplayText = fitTextToLcdCells (presetName.toString(), lcdCharacterLimit);

    if (static_cast<bool> (presetHasBeenEdited.getValue()))
    {
        if (presetDisplayText.length() >= lcdCharacterLimit)
            presetDisplayText = presetDisplayText.dropLastCharacters (1);

        presetDisplayText << "*";
    }
}

void MainMenu::drawLcdText (juce::Graphics& g,
    const juce::String& text,
    juce::Rectangle<float> area,
    const juce::Font& font) const
{
    if (text.isEmpty())
        return;

    g.setFont (font);
    const auto baselineY = std::floor (area.getY() + area.getHeight() - textBaselineInset);
    auto cursorX = std::floor (area.getX());

    const auto characterCount = static_cast<float> (jmax (1, text.length()));
    const auto cellWidth = lcdCharacterCellWidth > 0.0f
                               ? lcdCharacterCellWidth
                               : std::max (minimumCellWidth, std::floor (area.getWidth() / characterCount));

    for (const auto character : text)
    {
        const auto glyph = juce::String::charToString (character);
        const auto glyphArea = juce::Rectangle<float> (cursorX, area.getY(), cellWidth, area.getHeight());

        g.setColour (lcdTextColour);
        g.drawSingleLineText (glyph, roundToInt (glyphArea.getX()), roundToInt (baselineY));

        cursorX += cellWidth;
    }
}

juce::String MainMenu::fitTextToLcdCells (juce::String text, int maxCharacters) const
{
    text = text.trim().toUpperCase();

    if (text.length() <= maxCharacters)
        return text;

    return text.substring (0, maxCharacters);
}
