/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxComponent.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "EquinoxComponent.h"

//==============================================================================
EquinoxComponent::EquinoxComponent() {}

EquinoxComponent::~EquinoxComponent() {}

EquinoxSlider& EquinoxComponent::addEquinoxSlider (std::unique_ptr<EquinoxSlider> slider)
{
    auto& sliderRef = *slider;
    addAndMakeVisible (sliderRef);
    equinoxSliders.push_back (std::move (slider));
    return sliderRef;
}

EquinoxSlider* EquinoxComponent::getSlider (String parameterID) const
{
    for (const auto& slider : equinoxSliders)
    {
        if (slider->getID() == parameterID)
            return slider.get();
    }

    return nullptr;
}

void EquinoxComponent::drawTitle (Graphics& g, Colour textColour, String titleText, int x, int y, int width, int height)
{
    Rectangle<int> titleArea (x, y, width, height);
    g.setColour (textColour);
    g.drawText (titleText, titleArea, Justification::centredTop);
}

void EquinoxComponent::drawRectangle (Graphics& g,
    Colour rectangleColour,
    float x,
    float y,
    float width,
    float height,
    float cornerSize,
    float lineThickness)
{
    Rectangle<float> area (x, y, width, height);
    g.setColour (rectangleColour);
    g.drawRoundedRectangle (area, cornerSize, lineThickness);
}

void EquinoxComponent::setHorizontalSliderLayout (int labelYPos,
    int sliderYPos,
    unsigned int leftPadding,
    unsigned int rightPadding)
{
    Rectangle<int> localBounds = getLocalBounds();
    const int sliderCount = static_cast<int> (equinoxSliders.size());

    if (sliderCount == 0)
        return;

    const int horizontalSpace =
        (localBounds.getWidth() - static_cast<int> (rightPadding) - static_cast<int> (leftPadding)) / sliderCount;
    const int verticalSpace = localBounds.getHeight();

    const int labelWidth = horizontalSpace;
    const int labelHeight = 30;

    int xOffsetMultiplier = 0;

    for (auto& equinoxSlider : equinoxSliders)
    {
        equinoxSlider->setBounds (
            static_cast<int> (leftPadding) + (horizontalSpace * xOffsetMultiplier), 0, horizontalSpace, verticalSpace);

        equinoxSlider->setSliderComponentBounds (equinoxSlider->getSliderComponentBounds().getX(), sliderYPos);

        equinoxSlider->setLabelComponentBounds (
            equinoxSlider->getLabelComponentBounds().getX(), labelYPos, labelWidth, labelHeight);

        equinoxSlider->center();

        xOffsetMultiplier++;
    }
}
