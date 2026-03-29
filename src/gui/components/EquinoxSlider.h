/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    EquinoxSlider.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <functional>

//==============================================================================
/** Composite control that pairs a JUCE slider, label, and APVTS attachment. */
class EquinoxSlider : public juce::Component
{
public:
    /** Callback used to format the raw parameter value for popup display. */
    using TextFromValueFormatter = std::function<String (float rawValue)>;
    /** Callback used to parse popup text back into the raw parameter value. */
    using ValueFromTextParser = std::function<float (const String& text)>;

    /** Creates a slider with a label */
    EquinoxSlider (AudioProcessorValueTreeState& parameters,
        Slider::SliderStyle sliderStyle,
        float minSliderValue,
        float maxSliderValue,
        int sliderWidth,
        int sliderHeight,
        String paramID,
        String labelText,
        String valueSuffix = "",
        String valuePrefix = "",
        unsigned int valueMaxDecimals = 0,
        TextFromValueFormatter customTextFromValue = {},
        ValueFromTextParser customValueFromText = {});

    /** Destroys the slider and its APVTS attachment. */
    ~EquinoxSlider() override;

    /** Sets the position of the EquinoxSlider*/
    void setPosition (int x, int y);

    /** Sets the bounds of the Slider component*/
    void setSliderComponentBounds (int x, int y, int width = -1, int height = -1);

    /** Returns the bounds of the Slider component*/
    Rectangle<int> getSliderComponentBounds() const;

    /** Sets the bounds of the Label component*/
    void setLabelComponentBounds (int x, int y, int width = -1, int height = -1);

    /** Returns the bounds of the Label component*/
    Rectangle<int> getLabelComponentBounds() const;

    /** Returns the EquinoxSliders ID*/
    String getID() const;

    /** Returns a reference to the Slider component*/
    Slider& getSliderComponent();

    /** Returns a reference to the Label component*/
    Label& getLabelComponent();

    /** Centers the EquinoxSliders components*/
    void center();

    /** Paints any custom background for the composite control. */
    void paint (juce::Graphics&) override;
    /** Responds to component resizes. */
    void resized() override;

private:
    String parameterID;
    Slider slider;
    Label label;
    String suffix;
    String prefix;
    float minValue;
    float maxValue;
    unsigned int maxDecimals;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxSlider)
};
