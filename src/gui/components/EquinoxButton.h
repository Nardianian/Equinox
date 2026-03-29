/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    EquinoxButton.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/** Custom text button with Equinox-specific artwork and drop-shadow behavior. */
class EquinoxButton : public juce::TextButton
{
public:
    /** Visual styles supported by the custom Equinox button. */
    enum class Style
    {
        defaultButton,
        indicatorToggle
    };

    /** Creates a button and optionally enables toggle behavior. */
    EquinoxButton (String buttonName, bool shouldToggle = false, Style buttonStyle = Style::defaultButton);
    ~EquinoxButton() override = default;

    /** Paints the button using the Equinox visual style. */
    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    /** Creates the shadow helper used by the button. */
    void setShadower();

    /** Updates the shadow style to match the current button state. */
    void changeShadow();

    ButtonState lastState;

    bool isToggleButton = false;
    Style style = Style::defaultButton;

    DropShadow shadow;
    std::unique_ptr<DropShadower> shadower = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxButton)
};
