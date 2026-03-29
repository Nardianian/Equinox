/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxComponent.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "EquinoxSlider.h"
#include <JuceHeader.h>

//==============================================================================
/** Base component that manages Equinox sliders and common drawing helpers. */
class EquinoxComponent : public juce::Component
{
public:
    /** Creates an empty component with Equinox helper behavior. */
    EquinoxComponent();
    /** Destroys the component and any owned sliders. */
    ~EquinoxComponent() override;

    /** Adds a slider with a label to the component */
    EquinoxSlider& addEquinoxSlider (std::unique_ptr<EquinoxSlider> slider);

    /** Returns one of the sliders, or nullptr if the ID is not registered */
    EquinoxSlider* getSlider (String parameterID) const;

    /** Draws a title */
    void drawTitle (Graphics& g, Colour textColour, String titleText, int x, int y, int width, int height);

    /** Draws a rectangle */
    void drawRectangle (Graphics& g,
        Colour rectangleColour,
        float x,
        float y,
        float width,
        float height,
        float cornerSize = 1.0f,
        float lineThickness = 2.0f);

    /** Spreads the sliders evenly on a horizontal axis */
    void setHorizontalSliderLayout (int labelYPos,
        int sliderYPos,
        unsigned int leftPadding = 0,
        unsigned int rightPadding = 0);

private:
    std::vector<std::unique_ptr<EquinoxSlider>> equinoxSliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxComponent)
};
