/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxLookAndFeel.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** Global look-and-feel used by the Equinox editor. */
class EquinoxLookAndFeel : public LookAndFeel_V4
{
public:
    /** Creates the look-and-feel and loads the required image assets. */
    EquinoxLookAndFeel();

    /** Releases any loaded look-and-feel resources. */
    ~EquinoxLookAndFeel() override;

private:
    /** Draws the background image for Equinox buttons. */
    void drawButtonBackground (Graphics&,
        Button&,
        const Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    /** Draws button label text in the Equinox style. */
    void drawButtonText (Graphics& g,
        TextButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    /** Draws the custom linear slider style used in the UI. */
    void drawLinearSlider (Graphics&,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle,
        Slider&) override;

    /** Draws the custom rotary slider style used in the UI. */
    void drawRotarySlider (Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        const float rotaryStartAngle,
        const float rotaryEndAngle,
        Slider& slider) override;

    /** Draws the tab buttons used by the synth-layer selector. */
    void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown) override;

    /** Draws the area behind the front tab, optionally suppressed for custom rounded tab menus. */
    void drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, int w, int h) override;

    Image buttonUp;
    Image buttonDown;
    Image rotaryKnob;
    Image rotaryKnobShadow;
    Image sliderKnob;
    Image sliderLine;
};
