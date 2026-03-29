/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxButton.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "EquinoxButton.h"

namespace
{
    constexpr float indicatorOutlineThickness = 1.0f;
    constexpr float indicatorCornerSize = 6.0f;
    constexpr float indicatorOutlineAlpha = 0.8f;
    constexpr float indicatorHighlightAlpha = 0.18f;
    constexpr float indicatorGlowAlpha = 0.28f;
    constexpr int indicatorTextInset = 2;
    const auto indicatorOffFillTop = juce::Colour::fromRGBA (86, 92, 94, 90);
    const auto indicatorOffFillBottom = juce::Colour::fromRGBA (24, 28, 30, 150);
    const auto indicatorOffOutline = juce::Colour::fromRGBA (150, 160, 164, 125);
    const auto indicatorOnFillTop = juce::Colour::fromRGBA (100, 255, 170, 205);
    const auto indicatorOnFillBottom = juce::Colour::fromRGBA (18, 112, 72, 200);
    const auto indicatorOnOutline = juce::Colour::fromRGBA (175, 255, 215, 235);
    const auto indicatorTextColour = juce::Colour::fromRGB (18, 24, 20);
    const auto indicatorOffTextColour = juce::Colour::fromRGBA (210, 220, 214, 160);
}

//==============================================================================
EquinoxButton::EquinoxButton (String buttonName, bool shouldToggle, Style buttonStyle)
    : juce::TextButton (buttonName), shadow (Colours::black, 3, { 0, 1 })
{
    setShadower();
    onStateChange = [this]() { changeShadow(); };

    isToggleButton = shouldToggle;
    style = buttonStyle;

    setClickingTogglesState (shouldToggle);
}

void EquinoxButton::paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    if (style == Style::indicatorToggle)
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto isOn = getToggleState();
        const auto fillTop = isOn ? indicatorOnFillTop : indicatorOffFillTop;
        const auto fillBottom = isOn ? indicatorOnFillBottom : indicatorOffFillBottom;
        const auto outline = isOn ? indicatorOnOutline : indicatorOffOutline;
        const auto alpha = isEnabled() ? 1.0f : 0.5f;

        g.setGradientFill (
            juce::ColourGradient (fillTop.withMultipliedAlpha (alpha), bounds.getTopLeft(),
                fillBottom.withMultipliedAlpha (alpha), bounds.getBottomLeft(), false));
        g.fillRoundedRectangle (bounds, indicatorCornerSize);

        g.setColour (juce::Colours::white.withMultipliedAlpha (indicatorHighlightAlpha * alpha));
        g.drawRoundedRectangle (bounds.reduced (0.5f, 0.5f), indicatorCornerSize, indicatorOutlineThickness);

        g.setColour (outline.withMultipliedAlpha (indicatorOutlineAlpha * alpha));
        g.drawRoundedRectangle (bounds.reduced (0.5f, 0.5f), indicatorCornerSize, indicatorOutlineThickness);

        if (isOn)
        {
            g.setColour (indicatorOnOutline.withMultipliedAlpha (indicatorGlowAlpha * alpha));
            g.drawRoundedRectangle (bounds.expanded (0.5f), indicatorCornerSize + 0.5f, indicatorOutlineThickness);
        }

        const auto textBounds = getLocalBounds().reduced (indicatorTextInset, 0);
        g.setColour ((isOn ? indicatorTextColour : indicatorOffTextColour).withMultipliedAlpha (alpha));
        g.setFont (getLookAndFeel().getTextButtonFont (*this, getHeight()));
        g.drawFittedText (getButtonText(), textBounds, juce::Justification::centred, 1);
        return;
    }

    TextButton::paintButton (g, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void EquinoxButton::setShadower()
{
    shadower = std::make_unique<DropShadower> (shadow);
    shadower->setOwner (this);
}

void EquinoxButton::changeShadow()
{
    if (style == Style::indicatorToggle)
        return;

    /**Only change shadow on buttonDown or buttonNormal, or if button was released*/
    if (getState() != buttonOver || lastState == buttonDown)
    {
        if (getState() == buttonDown)
        {
            shadow.colour = Colours::transparentBlack;
        }
        else
        {
            shadow.colour = Colours::black;
            shadow.offset = { 0, 1 };
            shadow.radius = 3;
        }
        setShadower();
        lastState = getState();
    }
}
