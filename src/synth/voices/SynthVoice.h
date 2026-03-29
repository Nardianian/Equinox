/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|

    SynthVoice.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once

#include "synth/VoiceParameters.h"
#include "synth/dsp/AmpEnvelope.h"
#include "synth/dsp/Filter.h"
#include "synth/dsp/FilterEnvelope.h"
#include <JuceHeader.h>

/** Shared base class for Equinox oscillator and sampler voices. */
class SynthVoice : public SynthesiserVoice
{
public:
    /** Creates a voice and configures whether it should use mono behavior. */
    SynthVoice (bool isMonoVoice);

    virtual ~SynthVoice() override;

    /** Prepares the voices*/
    virtual void prepareVoice (const dsp::ProcessSpec&);

    /** Sets the stereo panning of the voice*/
    void setPanning (float panValue);

    /** Sets the voice amplitude*/
    void setAmplitude (float ampValue);

    /** Sets the voice detune*/
    void setDetune (float detuneValue);

    /** Sets the voice analog factor*/
    void setAnalogFactor (float analogFactorValue);

    /** Sets the voice pitch transpose*/
    void setPitchTranspose (float transposeValue);

    /** Sets the voice fine pitch*/
    void setFinePitch (float pitchValue);

    /** Sets the portamento time in milliseconds. */
    void setPortamento (float timeMilliseconds);

    /** Applies all shared per-voice parameters in one typed update. */
    void applyParameters (const VoiceParameters::Common& parameters);

    /** Returns true if mono is enabled*/
    [[nodiscard]] bool isMonoEnabled() const;

    /** Returns a reference to the voice filter object*/
    [[nodiscard]] Filter& getFilter();

    /** Returns a reference to the voice filter envelope object*/
    [[nodiscard]] FilterEnvelope& getFilterEnvelope();

    /** Returns a reference to the voice amp envelope object*/
    [[nodiscard]] AmpEnvelope& getAmpEnvelope();

    /** Resets the voice so preset/mode changes cannot leave stale playback state behind. */
    void resetState();

    /** Enables or disables mono behavior for this voice. */
    void setMonoMode (bool shouldBeMono) noexcept;

protected:
    /** Sets the base frequency for the voice. */
    void setBaseFrequency (double frequency);

    /** Returns the current frequency for the current sample. */
    [[nodiscard]] double getCurrentFrequency (double additionalRatio = 1.0) const noexcept;

    /** Advances the current portamento state after a sample has been rendered. */
    void advancePortamento() noexcept;

    /** Returns the key velocity*/
    [[nodiscard]] float getKeyVelocity() const;

    /** Returns the detune multiplier*/
    [[nodiscard]] double getDetuneRatio() const;

    /** Returns the amount of panning*/
    [[nodiscard]] float getPanning (int currentChannel) const;

    /** Returns a random offset in hertz based on the analog factor*/
    [[nodiscard]] float getAnalogPitch();

    /** Returns a random generated value set by the analog factor*/
    [[nodiscard]] float getRandomAnalogValue (bool positiveValuesOnly);

    /** Returns the current portamento ratio without advancing it. */
    [[nodiscard]] double getPortamentoRatio() const noexcept;

    /** Called when a note starts*/
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override;

    /** Called when a note ends*/
    void stopNote (float velocity, bool allowTailOff) override;

    /** Adds an audiobuffer to another audiobuffer*/
    void addBufferToOutput (AudioBuffer<float>& bufferToAdd,
        AudioBuffer<float>& outputBuffer,
        int startSample,
        int numSamples);

    /** Called when the pitch wheel has been moved*/
    void pitchWheelMoved (int newPitchWheelValue) override;

    /** Called when the controller has been moved*/
    void controllerMoved (int controllerNumber, int newControllerValue) override;

    /** Sets the amount of pitchbend*/
    void setPitchBend (int pitchWheelValue);

    /** Sets the frequency by midinote number*/
    void setBaseFrequencyFromMidiNote (int midiNote);

    /** Returns the current pitch bend in cents*/
    [[nodiscard]] float getPitchBendCents() const;

    /** Converts and returns the passed in midinote number and offset in cents as hertz*/
    [[nodiscard]] double noteInHertz (int midiNoteNumber, double centsOffset) const;

    /** Converts and returns a notes offset to hertz*/
    [[nodiscard]] double noteOffsetInHertz (double offset) const;

    static constexpr int leftChannelIndex = 0;
    static constexpr int rightChannelIndex = 1;

    AudioBuffer<float> voiceBuffer;

    int currentNoteNumber = -1;

    bool noteHasBeenTriggered = false;

    float currentVoiceAmplitude = 0;

    Filter voiceFilter;

    FilterEnvelope filterEnvelope;

    AmpEnvelope ampEnvelope;

private:
    /** Allows derived voices to clear any type-specific playback state on reset. */
    virtual void resetPlaybackState();

    friend class VoiceEngine;

    bool applyPortamento = false;

    double baseFrequency = 0.0;

    double previousFrequency = 0.0;

    double detuneRatio = 1.0;

    float masterAmplitude = 1.0f;

    float midiKeyVelocity = 0;

    double finePitchRatio = 1.0;

    double pitchTransposeRatio = 1.0;

    int analogFactor = 0;

    float panningValue = 0;

    double analogPitchRatio = 1.0;

    float pitchBendAmount = 0.0f;

    float pitchBendSemitones = 1.0f;

    double pitchBendRatio = 1.0;

    bool monoMode = false;

    static constexpr int defaultPortamentoSamples = 4000;

    int portamentoSamples = defaultPortamentoSamples;

    double portamentoRatio = 1.0;

    double portamentoDelta = 0.0;

    Random analogRandom;
};
