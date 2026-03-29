/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    TabMenu.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "TabMenu.h"

namespace
{
    [[nodiscard]] Rectangle<float> getContentAreaBounds (const TabMenu& menu)
    {
        auto contentBounds = menu.getLocalBounds();
        const auto tabBarBounds = menu.getTabbedButtonBar().getBounds();

        switch (menu.getOrientation())
        {
            case TabbedButtonBar::TabsAtTop:
                contentBounds.removeFromTop (tabBarBounds.getHeight());
                break;
            case TabbedButtonBar::TabsAtBottom:
                contentBounds.removeFromBottom (tabBarBounds.getHeight());
                break;
            case TabbedButtonBar::TabsAtLeft:
                contentBounds.removeFromLeft (tabBarBounds.getWidth());
                break;
            case TabbedButtonBar::TabsAtRight:
                contentBounds.removeFromRight (tabBarBounds.getWidth());
                break;
            default:
                break;
        }

        return contentBounds.toFloat();
    }
}

TabMenu::TabMenu (TabbedButtonBar::Orientation orientation) : TabbedComponent (orientation)
{
    onTabChanged = []() {};
}

void TabMenu::setOutlineStyle (OutlineStyle newStyle)
{
    outlineStyle = newStyle;
    setOutline (outlineStyle == OutlineStyle::native ? 1 : 0);
    repaint();
}

TabMenu::OutlineStyle TabMenu::getOutlineStyle() const noexcept
{
    return outlineStyle;
}

void TabMenu::setRoundedOutlineAppearance (Colour colour, float cornerSize, float thickness)
{
    roundedOutlineColour = colour;
    roundedOutlineCornerSize = cornerSize;
    roundedOutlineThickness = thickness;
    repaint();
}

void TabMenu::currentTabChanged (int currentTabIndex, const String& currentTabName)
{
    ignoreUnused (currentTabIndex, currentTabName);
    onTabChanged();
}

void TabMenu::paintOverChildren (Graphics& g)
{
    TabbedComponent::paintOverChildren (g);

    if (outlineStyle != OutlineStyle::rounded)
        return;

    if (getNumTabs() > 0)
    {
        auto outlineBounds = getContentAreaBounds (*this).reduced (roundedOutlineThickness * 0.5f);
        const auto clampedCornerSize =
            jmin (roundedOutlineCornerSize, 0.5f * jmin (outlineBounds.getWidth(), outlineBounds.getHeight()));

        Path outlinePath;
        outlinePath.addRoundedRectangle (outlineBounds.getX(),
            outlineBounds.getY(),
            outlineBounds.getWidth(),
            outlineBounds.getHeight(),
            clampedCornerSize,
            clampedCornerSize,
            getOrientation() != TabbedButtonBar::TabsAtTop,
            true,
            false,
            true);

        Graphics::ScopedSaveState state (g);

        if (auto* selectedTabButton = getTabbedButtonBar().getTabButton (getCurrentTabIndex()))
        {
            auto gapBounds = selectedTabButton->getBounds().toFloat();
            const auto tabBarBounds = getTabbedButtonBar().getBounds().toFloat();
            gapBounds = gapBounds.translated (tabBarBounds.getX(), tabBarBounds.getY());
            const auto gapInset = roundedOutlineThickness;

            const Rectangle<float> gapClipBounds (gapBounds.getX() + gapInset,
                outlineBounds.getY() - roundedOutlineThickness,
                jmax (0.0f, gapBounds.getWidth() - (gapInset * 2.0f)),
                roundedOutlineThickness * 3.0f);

            g.excludeClipRegion (gapClipBounds.getSmallestIntegerContainer());
        }

        g.setColour (roundedOutlineColour);
        g.strokePath (outlinePath, PathStrokeType (roundedOutlineThickness));
    }
}

void TabMenu::resized()
{
    TabbedComponent::resized();

    if (outlineStyle == OutlineStyle::rounded)
        repaint();
}
