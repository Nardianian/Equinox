/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxLookAndFeel.cpp
    Author:  Oliver Rasmussen
    
  ==============================================================================
*/

#include "EquinoxLookAndFeel.h"
#include "TabMenu.h"
#include <GraphicsBinaryData.h>

namespace
{
    constexpr float pressedButtonWidthMultiplier = 0.98f;
    constexpr float pressedButtonTextDarkening = 0.15f;
    constexpr float buttonDisabledAlpha = 0.5f;
    constexpr float buttonHoverAlpha = 1.0f;
    constexpr float buttonIdleAlpha = 0.8f;
    constexpr float buttonMinAlpha = 0.3f;
    constexpr float buttonTopGradientBoost = 0.2f;
    constexpr float buttonBottomGradientDarkening = 0.1f;
    constexpr int maxButtonTextLines = 2;
    constexpr int maxVerticalIndent = 4;
    constexpr float verticalIndentProportion = 0.3f;
    constexpr float buttonFontScale = 0.6f;
    constexpr float rotaryInset = 15.0f;
    constexpr float rotaryShadowOffsetY = 5.0f;
    constexpr float rotaryKnobOffsetY = 7.0f;
    constexpr float sliderCentreFactor = 0.5f;
    constexpr float sliderLineYOffset = -10.0f;
    constexpr float sliderLineExtraHeight = 17.0f;

    [[nodiscard]] float asFloat (int value) noexcept { return static_cast<float> (value); }
} // namespace

EquinoxLookAndFeel::EquinoxLookAndFeel()
{
    rotaryKnob = ImageCache::getFromMemory (GraphicsResources::RotaryKnob_png, GraphicsResources::RotaryKnob_pngSize);
    rotaryKnobShadow = ImageCache::getFromMemory (
        GraphicsResources::RotaryKnobShadow_png, GraphicsResources::RotaryKnobShadow_pngSize);

    sliderKnob =
        ImageCache::getFromMemory (GraphicsResources::SmallSliderKnob_png, GraphicsResources::SmallSliderKnob_pngSize);
    sliderLine = ImageCache::getFromMemory (GraphicsResources::SliderLine_png, GraphicsResources::SliderLine_pngSize);

    buttonUp = ImageCache::getFromMemory (GraphicsResources::BtnUp1_png, GraphicsResources::BtnUp1_pngSize);
    buttonDown = ImageCache::getFromMemory (GraphicsResources::BtnDown1_png, GraphicsResources::BtnDown1_pngSize);
}

EquinoxLookAndFeel::~EquinoxLookAndFeel() {}

void EquinoxLookAndFeel::drawButtonBackground (Graphics& g,
    Button& button,
    const Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    ignoreUnused (button, backgroundColour, shouldDrawButtonAsHighlighted);
    auto btnBounds = button.getLocalBounds().toFloat();

    Image btnBackground = shouldDrawButtonAsDown ? buttonDown : buttonUp;

    g.drawImage (btnBackground, btnBounds);
}

void EquinoxLookAndFeel::drawButtonText (Graphics& g,
    TextButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    ignoreUnused (shouldDrawButtonAsHighlighted);
    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);

    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId : TextButton::textColourOffId)
            .withMultipliedAlpha (button.isEnabled() ? buttonHoverAlpha : buttonDisabledAlpha));

    float widthMultiplier = 1.0f;
    float xPosMultiplier = 1.0f;

    if (shouldDrawButtonAsDown)
    {
        widthMultiplier = pressedButtonWidthMultiplier;
        xPosMultiplier += 1.0f - widthMultiplier;
        g.setColour (button.findColour (TextButton::textColourOnId).darker (pressedButtonTextDarkening));
        g.setColour (button.findColour (TextButton::textColourOffId).darker (pressedButtonTextDarkening));
    }

    const int yIndent = jmin (maxVerticalIndent, button.proportionOfHeight (verticalIndentProportion));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * buttonFontScale);
    const float leftIndent =
        static_cast<float> (jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2)));
    const float rightIndent =
        static_cast<float> (jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2)));
    const float buttonWidth = asFloat (button.getWidth());
    const float textWidth = (buttonWidth - leftIndent - rightIndent) * widthMultiplier;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
            roundToInt (leftIndent * xPosMultiplier),
            yIndent,
            roundToInt (textWidth),
            button.getHeight() - (yIndent * 2),
            Justification::centred,
            maxButtonTextLines);
}

void EquinoxLookAndFeel::drawLinearSlider (Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    const Slider::SliderStyle style,
    Slider& slider)
{
    ignoreUnused (minSliderPos, maxSliderPos, style, slider);
    const float lineCentreX =
        asFloat (x) + (asFloat (width) * sliderCentreFactor) - (asFloat (sliderLine.getWidth()) * sliderCentreFactor);

    const float knobCentreX =
        asFloat (x) + (asFloat (width) * sliderCentreFactor) - (asFloat (sliderKnob.getWidth()) * sliderCentreFactor);

    const float knobCentreY = asFloat (sliderKnob.getHeight()) * sliderCentreFactor;

    const float knobPos = sliderPos - knobCentreY;

    Rectangle<float> sliderLineRect (lineCentreX,
        asFloat (y) + sliderLineYOffset,
        asFloat (sliderLine.getWidth()),
        asFloat (height) + sliderLineExtraHeight);
    g.drawImage (sliderLine, sliderLineRect);

    g.drawImageAt (sliderKnob, roundToInt (knobCentreX), roundToInt (knobPos));
}

void EquinoxLookAndFeel::drawRotarySlider (Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    const float rotaryStartAngle,
    const float rotaryEndAngle,
    Slider& slider)
{
    ignoreUnused (x, y, slider);
    Image knob =
        rotaryKnob.rescaled (roundToInt (asFloat (width) - rotaryInset), roundToInt (asFloat (height) - rotaryInset));

    const float centreX = asFloat (width) * sliderCentreFactor;

    const float rotaryKnobCentreX = asFloat (knob.getWidth()) * sliderCentreFactor;
    const float rotaryKnobCentreY = asFloat (knob.getHeight()) * sliderCentreFactor;
    const float rotation =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle) - MathConstants<float>::pi;

    AffineTransform transform;
    const float knobPosX = centreX - rotaryKnobCentreX;
    const float knobPosY = rotaryKnobOffsetY;

    transform = transform.rotation (rotation, rotaryKnobCentreX, rotaryKnobCentreY).translated (knobPosX, knobPosY);

    g.drawImage (rotaryKnobShadow,
        { knobPosX, knobPosY + rotaryShadowOffsetY, asFloat (knob.getWidth()), asFloat (knob.getHeight()) });

    g.drawImageTransformed (knob, transform, false);
}

void EquinoxLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());

    const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

    //const Colour bkg (button.getTabBackgroundColour());

    const Colour bkg (Colours::black);

    if (button.getToggleState())
    {
        g.setColour (bkg);
    }
    else
    {
        Point<int> p1, p2;

        switch (o)
        {
            case TabbedButtonBar::TabsAtBottom:
                p1 = activeArea.getBottomLeft();
                p2 = activeArea.getTopLeft();
                break;
            case TabbedButtonBar::TabsAtTop:
                p1 = activeArea.getTopLeft();
                p2 = activeArea.getBottomLeft();
                break;
            case TabbedButtonBar::TabsAtRight:
                p1 = activeArea.getTopRight();
                p2 = activeArea.getTopLeft();
                break;
            case TabbedButtonBar::TabsAtLeft:
                p1 = activeArea.getTopLeft();
                p2 = activeArea.getTopRight();
                break;
            default:
                jassertfalse;
                break;
        }

        g.setGradientFill (ColourGradient (bkg.brighter (buttonTopGradientBoost),
            p1.toFloat(),
            bkg.darker (buttonBottomGradientDarkening),
            p2.toFloat(),
            false));
    }

    g.fillRect (activeArea);

    g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));

    Rectangle<int> r (activeArea);

    if (o != TabbedButtonBar::TabsAtBottom)
        g.fillRect (r.removeFromTop (1));
    if (o != TabbedButtonBar::TabsAtTop)
        g.fillRect (r.removeFromBottom (1));
    if (o != TabbedButtonBar::TabsAtRight)
        g.fillRect (r.removeFromLeft (1));
    if (o != TabbedButtonBar::TabsAtLeft)
        g.fillRect (r.removeFromRight (1));

    const float alpha =
        button.isEnabled() ? ((isMouseOver || isMouseDown) ? buttonHoverAlpha : buttonIdleAlpha) : buttonMinAlpha;

    Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
    {
        TabbedButtonBar::ColourIds colID =
            button.isFrontTab() ? TabbedButtonBar::frontTextColourId : TabbedButtonBar::tabTextColourId;

        if (bar->isColourSpecified (colID))
            col = bar->findColour (colID);
        else if (isColourSpecified (colID))
            col = findColour (colID);
    }

    const Rectangle<float> area (button.getTextArea().toFloat());

    float length = area.getWidth();
    float depth = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);

    AffineTransform t;

    switch (o)
    {
        case TabbedButtonBar::TabsAtLeft:
            t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom());
            break;
        case TabbedButtonBar::TabsAtRight:
            t = t.rotated (MathConstants<float>::pi * 0.5f).translated (area.getRight(), area.getY());
            break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom:
            t = t.translated (area.getX(), area.getY());
            break;
        default:
            jassertfalse;
            break;
    }

    g.addTransform (t);
    textLayout.draw (g, Rectangle<float> (length, depth));
}

void EquinoxLookAndFeel::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, int w, int h)
{
    if (auto* tabMenu = dynamic_cast<TabMenu*> (bar.getParentComponent()))
        if (tabMenu->getOutlineStyle() == TabMenu::OutlineStyle::rounded)
            return;

    LookAndFeel_V4::drawTabAreaBehindFrontButton (bar, g, w, h);
}
