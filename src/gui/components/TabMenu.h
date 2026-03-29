/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    TabMenu.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** TabbedComponent wrapper that exposes a simple callback when the selected tab changes. */
class TabMenu : public TabbedComponent
{
public:
    /** Available outline styles for the tab content area. */
    enum class OutlineStyle
    {
        native,
        none,
        rounded
    };

    /** Creates the tab menu with the requested tab-strip orientation. */
    TabMenu (TabbedButtonBar::Orientation orientation);

    /** Selects how the tab content outline should be drawn. */
    void setOutlineStyle (OutlineStyle newStyle);

    /** Returns the current custom outline mode. */
    [[nodiscard]] OutlineStyle getOutlineStyle() const noexcept;

    /** Sets the colour and geometry used by the rounded custom outline. */
    void setRoundedOutlineAppearance (Colour colour, float cornerSize = 20.0f, float thickness = 2.0f);

    /** Broadcasts tab changes to the optional callback. */
    void currentTabChanged (int currentTabIndex, const String& currentTabName) override;

    /** Draws any custom outline over the active content component. */
    void paintOverChildren (Graphics& g) override;

    /** Repaints the custom outline after JUCE updates the tab content bounds. */
    void resized() override;

    /** Optional callback fired whenever the selected tab changes. */
    std::function<void()> onTabChanged;

private:
    OutlineStyle outlineStyle { OutlineStyle::native };
    Colour roundedOutlineColour { Colours::darkgrey };
    float roundedOutlineCornerSize { 20.0f };
    float roundedOutlineThickness { 2.0f };
};
