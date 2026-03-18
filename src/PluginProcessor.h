#pragma once

#include "EquinoxSynthesizer.h"
#include "MasterEffectChain.h"
#include "PresetManager.h"
#include "StateManager.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class EquinoxAudioProcessor : public AudioProcessor
{
public:
    //==============================================================================
    EquinoxAudioProcessor();
    ~EquinoxAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    EquinoxSynthesizer synthLayer1, synthLayer2, synthLayer3;

    MasterEffectChain masterEffectChain;

    StateManager stateManager;

    PresetManager presetManager;

    MidiKeyboardState midiKeyboardState;

private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout();

    /**
        * @brief Validates the audio buffer to ensure no NaN, Inf, or out-of-range values are present.
        * Useful for debugging to protect against unexpected audio values on the output.
        * @param buffer The audio buffer to validate.
        * @param minValue The minimum value allowed in the buffer. Default is -1.0f.
        * @param maxValue The maximum value allowed in the buffer. Default is 1.0f.
     */
    void validateAudioBuffer (const juce::AudioBuffer<float>& buffer, float minValue = -1.0f, float maxValue = 1.0f)
    {
        // Get the number of channels and samples
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelData = buffer.getReadPointer (channel);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                // Assert if NaN, Inf, or out-of-range values are found
                jassert (!std::isnan (channelData[sample])); // Ensure no NaN values
                jassert (!std::isinf (channelData[sample])); // Ensure no Inf values
                jassert (channelData[sample] >= minValue && channelData[sample] <= maxValue); // Ensure values are within range
            }
        }
    }

    double lastSampleRate;
    double currentBPM = 120;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EquinoxAudioProcessor)
};
