#include "state/ParameterIds.h"
#include "state/ParameterModels.h"
#include "state/PluginParameterLayout.h"
#include "state/PluginState.h"
#include "state/PresetManager.h"
#include "synth/dsp/AmpEnvelope.h"
#include "synth/dsp/Filter.h"
#include "synth/dsp/FilterEnvelope.h"
#include "synth/dsp/WavetableOscillator.h"
#include "synth/voices/OscSynthSound.h"
#include "synth/voices/OscSynthVoice.h"
#include "synth/voices/SampleSynthSound.h"
#include "synth/voices/SampleSynthVoice.h"
#include "utils/OutputSafetyProtector.h"

namespace
{
    struct TestFailure
    {
        String message;
    };

    class TestProcessor : public AudioProcessor
    {
    public:
        TestProcessor() : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)) {}

        const String getName() const override { return "EquinoxCoreTests"; }
        void prepareToPlay (double, int) override {}
        void releaseResources() override {}
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override
        {
            return layouts.getMainOutputChannelSet() == AudioChannelSet::stereo();
        }

        void processBlock (AudioBuffer<float>&, MidiBuffer&) override {}
        AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const String getProgramName (int) override { return {}; }
        void changeProgramName (int, const String&) override {}
        void getStateInformation (MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}
    };

    struct TestContext
    {
        TestProcessor processor;
        AudioProcessorValueTreeState parameters;
        AudioSampleValueTreeState audioSamples;
        PluginState pluginState;

        TestContext()
            : parameters (processor, nullptr, "parameterstate", createPluginParameterLayout (3)),
              audioSamples ("audiosamplestate"),
              pluginState (parameters, audioSamples)
        {
        }
    };

    void expect (bool condition, const String& message, Array<TestFailure>& failures)
    {
        if (!condition)
            failures.add ({ message });
    }

    void setFloatParameter (AudioProcessorValueTreeState& parameters, const String& parameterId, float value)
    {
        auto parameterValue = parameters.getParameterAsValue (parameterId);
        parameterValue = value;
    }

    bool valueAsBool (Value* value) { return value != nullptr && static_cast<bool> (value->getValue()); }

    class TestOscVoice : public OscSynthVoice
    {
    public:
        TestOscVoice() : OscSynthVoice (false) {}

        using SynthVoice::advancePortamento;
        using SynthVoice::getCurrentFrequency;
    };

    class TestSampleVoice : public SampleSynthVoice
    {
    public:
        TestSampleVoice() : SampleSynthVoice (false) {}

        using SynthVoice::getCurrentFrequency;
    };

    void testParameterIds (Array<TestFailure>& failures)
    {
        const auto parameterIds = ParameterIds::allParameterIds (3);
        StringArray sortedIds (parameterIds);
        sortedIds.sort (true);

        expect (parameterIds.size() == 95, "Expected 95 plugin parameter ids", failures);

        for (int index = 1; index < sortedIds.size(); ++index)
            expect (sortedIds[index - 1] != sortedIds[index], "Parameter ids must be unique", failures);

        expect (
            ParameterIds::Synth (2).ampAttack == "ampAttack2", "Synth layer ids should be instance-specific", failures);
        expect (ParameterIds::Synth (3).sampleStateId == "audiosample3",
            "Sample state ids should match synth layers",
            failures);
    }

    void testPluginStateRoundTrip (Array<TestFailure>& failures)
    {
        TestContext context;
        const auto synthIds = ParameterIds::Synth (1);

        setFloatParameter (context.parameters, synthIds.amplitude, 0.42f);
        setFloatParameter (context.parameters, synthIds.cutoff, 1234.0f);
        context.pluginState.setPresetName ("RoundTrip");

        MemoryBlock serializedState;
        context.pluginState.saveStateToBinary (serializedState);

        setFloatParameter (context.parameters, synthIds.amplitude, 0.11f);
        context.pluginState.setPresetName ("Modified");
        context.pluginState.loadStateFromBinary (serializedState.getData(), static_cast<int> (serializedState.getSize()));

        expect (approximatelyEqual (context.parameters.getRawParameterValue (synthIds.amplitude)->load(), 0.42f),
            "Binary round-trip should restore parameter values",
            failures);
        expect (context.pluginState.getPresetName().toString() == "RoundTrip",
            "Binary round-trip should restore preset metadata",
            failures);
        expect (context.pluginState.getRestorationVersion() > 0,
            "Successful restore should increment restoration version",
            failures);
    }

    void testPluginStateRejectsMalformedXml (Array<TestFailure>& failures)
    {
        TestContext context;
        const auto synthIds = ParameterIds::Synth (1);

        setFloatParameter (context.parameters, synthIds.amplitude, 0.64f);
        context.pluginState.setPresetName ("ValidPreset");

        auto malformedState = std::make_unique<XmlElement> ("currentstate");
        malformedState->setAttribute ("presetname", "BrokenPreset");

        if (auto parameterStateXml = context.parameters.copyState().createXml())
        {
            malformedState->addChildElement (new XmlElement (*parameterStateXml));
            malformedState->addChildElement (parameterStateXml.release());
        }

        if (auto audioSampleStateXml = context.audioSamples.state.createCopy().createXml())
            malformedState->addChildElement (audioSampleStateXml.release());

        MemoryBlock malformedBinary;
        AudioPluginInstance::copyXmlToBinary (*malformedState, malformedBinary);
        context.pluginState.loadStateFromBinary (malformedBinary.getData(), static_cast<int> (malformedBinary.getSize()));

        expect (approximatelyEqual (context.parameters.getRawParameterValue (synthIds.amplitude)->load(), 0.64f),
            "Malformed state should not mutate parameters",
            failures);
        expect (context.pluginState.getPresetName().toString() == "ValidPreset",
            "Malformed state should not mutate preset metadata",
            failures);
    }

    void testPresetManagerWorkflow (Array<TestFailure>& failures)
    {
        TestContext context;
        const auto synthIds = ParameterIds::Synth (1);
        const auto tempDirectory = File::getCurrentWorkingDirectory()
                                       .getChildFile (".equinox-test-tmp")
                                       .getChildFile ("EquinoxCoreTests_" + Uuid().toString());
        tempDirectory.createDirectory();

        {
            PresetManager presetManager (context.pluginState, 3, tempDirectory);
            const auto presetFile = tempDirectory.getChildFile ("WorkflowTest.equinox");

            setFloatParameter (context.parameters, synthIds.amplitude, 0.31f);
            context.pluginState.setPresetName ("WorkflowTest");

            expect (presetManager.savePresetToFile (presetFile), "Preset save should succeed", failures);
            expect (presetManager.currentPresetExistsAsFile(),
                "Saved preset should become the current preset file",
                failures);
            expect (!valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Saving should clear edited state",
                failures);

            if (auto savedStateXml = XmlDocument (presetFile).getDocumentElement())
            {
                expect (!savedStateXml->hasAttribute ("presetname"),
                    "Preset files should not serialize presetname metadata",
                    failures);
            }
            else
            {
                expect (false, "Preset save should produce valid XML", failures);
            }

            setFloatParameter (context.parameters, synthIds.amplitude, 0.77f);
            expect (valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Parameter edits should set the preset dirty flag",
                failures);

            context.pluginState.setPresetName ("SomethingElse");
            expect (presetManager.loadPresetFromFile (presetFile), "Preset load should succeed", failures);
            expect (approximatelyEqual (context.parameters.getRawParameterValue (synthIds.amplitude)->load(), 0.31f),
                "Preset load should restore saved parameter values",
                failures);
            expect (context.pluginState.getPresetName().toString() == "WorkflowTest",
                "Preset file loads should derive the preset name from the file name",
                failures);
            expect (!valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Preset load should clear edited state",
                failures);
        }

        tempDirectory.deleteRecursively();
    }

    void testPresetManagerRestoreSync (Array<TestFailure>& failures)
    {
        TestContext context;
        const auto synthIds = ParameterIds::Synth (1);
        const auto tempDirectory = File::getCurrentWorkingDirectory()
                                       .getChildFile (".equinox-test-tmp")
                                       .getChildFile ("EquinoxRestoreTests_" + Uuid().toString());
        tempDirectory.createDirectory();

        {
            PresetManager presetManager (context.pluginState, 3, tempDirectory);
            const auto presetFile = tempDirectory.getChildFile ("RestoreTest.equinox");

            setFloatParameter (context.parameters, synthIds.amplitude, 0.45f);
            context.pluginState.setPresetName ("RestoreTest");
            expect (presetManager.savePresetToFile (presetFile), "Restore sync preset save should succeed", failures);

            MemoryBlock serializedState;
            context.pluginState.saveStateToBinary (serializedState);

            setFloatParameter (context.parameters, synthIds.amplitude, 0.73f);
            expect (valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Parameter edits before restore should set the dirty flag",
                failures);

            context.pluginState.loadStateFromBinary (
                serializedState.getData(), static_cast<int> (serializedState.getSize()));
            presetManager.syncAfterExternalStateRestore (true);

            expect (!valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Restoring state that matches the backing preset file should clear the dirty flag",
                failures);

            expect (presetFile.deleteFile(), "Restore sync test should be able to delete the preset file", failures);

            context.pluginState.loadStateFromBinary (
                serializedState.getData(), static_cast<int> (serializedState.getSize()));
            presetManager.syncAfterExternalStateRestore (true);

            expect (valueAsBool (presetManager.getCurrentPresetHasBeenEdited()),
                "Restoring state without a backing preset file should leave the preset marked as unsaved",
                failures);
            expect (!presetManager.currentPresetExistsAsFile(),
                "Deleted preset files should no longer count as the current preset file",
                failures);
        }

        tempDirectory.deleteRecursively();
    }

    void testOutputSafetyProtector (Array<TestFailure>& failures)
    {
        OutputSafetyProtector protector;
        protector.setParameters ({ .ceilingDb = -0.1f,
            .emergencyThresholdLinear = 3.0f,
            .dcBlockCutoffHz = 20.0f,
            .muteTimeMs = 2.0f,
            .recoveryTimeMs = 2.0f });
        protector.prepare ({ 1000.0, 8, 2 });

        AudioBuffer<float> buffer (2, 8);
        buffer.clear();

        buffer.setSample (0, 0, 4.0f);
        buffer.setSample (1, 0, 4.0f);
        buffer.setSample (0, 1, 0.8f);
        buffer.setSample (1, 1, 0.8f);
        buffer.setSample (0, 2, 0.8f);
        buffer.setSample (1, 2, 0.8f);
        buffer.setSample (0, 3, 0.8f);
        buffer.setSample (1, 3, 0.8f);
        buffer.setSample (0, 4, 2.0f);
        buffer.setSample (1, 4, -2.0f);
        buffer.setSample (0, 5, 0.8f);
        buffer.setSample (1, 5, 0.8f);
        buffer.setSample (0, 6, 0.8f);
        buffer.setSample (1, 6, 0.8f);
        buffer.setSample (0, 7, 0.8f);
        buffer.setSample (1, 7, 0.8f);

        protector.process (buffer);

        expect (approximatelyEqual (buffer.getSample (0, 0), 0.0f),
            "Emergency samples should be muted immediately",
            failures);
        expect (approximatelyEqual (buffer.getSample (0, 1), 0.0f) && approximatelyEqual (buffer.getSample (0, 2), 0.0f),
            "Protection should hold a short mute after a runaway sample",
            failures);
        expect (buffer.getSample (0, 3) > 0.0f && buffer.getSample (0, 3) < 0.8f,
            "Protection should recover smoothly after the mute hold",
            failures);
        expect (std::abs (buffer.getSample (0, 4)) <= Decibels::decibelsToGain (-0.1f) + 1.0e-4f,
            "Large but finite peaks should be clamped to the safety ceiling",
            failures);
        expect (!protector.isProtectionActive(), "Protection should return to its idle state after recovery", failures);
    }

    void testOutputSafetyProtectorDcBlock (Array<TestFailure>& failures)
    {
        OutputSafetyProtector protector;
        protector.setParameters ({ .ceilingDb = -0.1f,
            .emergencyThresholdLinear = 10.0f,
            .dcBlockCutoffHz = 20.0f,
            .muteTimeMs = 2.0f,
            .recoveryTimeMs = 2.0f });
        protector.prepare ({ 1000.0, 64, 2 });

        AudioBuffer<float> buffer (2, 64);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                buffer.setSample (channel, sample, 0.5f);

        protector.process (buffer);

        expect (
            std::abs (buffer.getSample (0, 0)) > 0.1f, "DC blocker should allow the initial transient through", failures);
        expect (std::abs (buffer.getSample (0, buffer.getNumSamples() - 1)) < 0.05f,
            "DC blocker should attenuate steady DC offsets over time",
            failures);
        expect (
            !protector.isProtectionActive(), "DC offset removal alone should not trip emergency protection", failures);
    }

    void testFilterClampsUnsafeCutoffValues (Array<TestFailure>& failures)
    {
        Filter filter;
        dsp::ProcessSpec spec { 48000.0, 64, 2 };
        filter.prepareToPlay (spec);

        VoiceParameters::Filter filterParameters;
        filterParameters.cutoff = 0.0f;
        filterParameters.resonance = 0.0f;
        filterParameters.drive = 1.0f;

        filter.setFilter (filterParameters);
        expect (filter.getCutoffValue() >= 5.0f, "Filter should clamp cutoff values away from zero", failures);

        filter.modulateCutoff (0.0f);
        filter.restoreCutoff();
        expect (filter.getCutoffValue() >= 5.0f, "Filter restore path should keep a safe cutoff value", failures);
    }

    void testFilterDefaultsAreSafe (Array<TestFailure>& failures)
    {
        VoiceParameters::Filter filterDefaults;

        expect (filterDefaults.cutoff > 0.0f && filterDefaults.drive >= 1.0f,
            "Voice filter defaults should start from safe runtime values",
            failures);
    }

    void testAmpEnvelopeHandlesZeroAttackAndRelease (Array<TestFailure>& failures)
    {
        AmpEnvelope envelope;
        envelope.prepareToPlay (1000.0);
        envelope.setEnvelope ({ .attack = 0.0f, .decay = 0.0f, .sustain = 1.0f, .release = 0.0f });
        envelope.noteOn();

        const auto firstSample = envelope.getNextSample();
        expect (std::isfinite (firstSample) && firstSample >= 0.0f,
            "Amp envelope should produce a finite sample with zero attack",
            failures);

        envelope.noteOff();

        int safetyCounter = 0;
        while (envelope.isActive() && safetyCounter++ < 32)
            envelope.getNextSample();

        expect (
            !envelope.isActive(), "Amp envelope should finish release even when configured with zero release", failures);
    }

    void testFilterEnvelopeEnableDisableBehavior (Array<TestFailure>& failures)
    {
        Filter filter;
        dsp::ProcessSpec spec { 48000.0, 64, 2 };
        filter.prepareToPlay (spec);

        VoiceParameters::Filter filterParameters;
        filterParameters.cutoff = 1200.0f;
        filterParameters.drive = 1.0f;
        filter.setFilter (filterParameters);

        FilterEnvelope envelope (filter);
        envelope.prepareToPlay (1000.0);
        envelope.setCutoffLimit (8000.0f);

        envelope.setEnvelope ({});
        envelope.noteOn();
        envelope.calculateNextValue();
        expect (!envelope.isActive(), "A zeroed filter envelope should stay disabled", failures);

        envelope.setEnvelope ({ .attack = 0.0f, .decay = 0.01f, .sustain = 0.0f, .release = 0.01f });
        envelope.noteOn();
        expect (envelope.isActive(), "A non-zero filter envelope should become active on note-on", failures);

        envelope.calculateNextValue();
        envelope.noteOff();

        int safetyCounter = 0;
        while (envelope.isActive() && safetyCounter++ < 64)
            envelope.calculateNextValue();

        expect (!envelope.isActive(), "Filter envelope should return to an inactive state after release", failures);
    }

    void testVoiceSoundTypePairing (Array<TestFailure>& failures)
    {
        OscSynthVoice oscillatorVoice (false);
        SampleSynthVoice samplerVoice (false);
        OscSynthSound oscillatorSound;
        SampleSynthSound samplerSound;

        expect (oscillatorVoice.canPlaySound (&oscillatorSound),
            "Oscillator voices should accept oscillator sounds",
            failures);
        expect (
            !oscillatorVoice.canPlaySound (&samplerSound), "Oscillator voices should reject sampler sounds", failures);
        expect (samplerVoice.canPlaySound (&samplerSound), "Sampler voices should accept sampler sounds", failures);
        expect (
            !samplerVoice.canPlaySound (&oscillatorSound), "Sampler voices should reject oscillator sounds", failures);
    }

    void testOscillatorVoicePortamentoIsSideEffectFree (Array<TestFailure>& failures)
    {
        TestOscVoice voice;
        OscSynthSound sound;
        dsp::ProcessSpec spec { 48000.0, 64, 2 };
        VoiceParameters::Common parameters;
        parameters.amplitude = 1.0f;
        parameters.analogValue = 1.0f;
        parameters.portamento = 16.0f;

        WavetableOscillator::prepare (spec.sampleRate);
        voice.prepareVoice (spec);
        voice.applyParameters (parameters);
        voice.applyOscillatorParameters ({ .waveform = 0.0f, .phase = 0.0f });
        voice.startNote (60, 1.0f, &sound, 8192);
        voice.startNote (72, 1.0f, &sound, 8192);

        const auto firstFrequency = voice.getCurrentFrequency();
        const auto secondFrequency = voice.getCurrentFrequency();
        expect (approximatelyEqual (static_cast<float> (firstFrequency), static_cast<float> (secondFrequency)),
            "Frequency reads should be side-effect free before explicit portamento advancement",
            failures);

        voice.advancePortamento();

        const auto advancedFrequency = voice.getCurrentFrequency();
        expect (!approximatelyEqual (static_cast<float> (advancedFrequency), static_cast<float> (firstFrequency)),
            "Explicit portamento advancement should move the current frequency",
            failures);
    }

    void testSampleVoiceFrequencyTracksRootNote (Array<TestFailure>& failures)
    {
        TestSampleVoice voice;
        dsp::ProcessSpec spec { 48000.0, 64, 2 };
        VoiceParameters::Common parameters;
        parameters.amplitude = 1.0f;
        parameters.analogValue = 1.0f;
        voice.prepareVoice (spec);
        voice.applyParameters (parameters);

        AudioBuffer<float> sampleBuffer (2, 64);
        sampleBuffer.clear();
        auto sample = std::make_shared<const AudioSample> (
            "TestSample", "wav", 48000, 16, 2, sampleBuffer.getNumSamples(), sampleBuffer);
        SampleSynthSound sound (sample, 60);

        voice.startNote (72, 1.0f, &sound, 8192);

        expect (approximatelyEqual (static_cast<float> (voice.getCurrentFrequency()), 2.0f),
            "Sampler frequency should double one octave above the root note",
            failures);
    }
} // namespace

int main()
{
    Array<TestFailure> failures;

    MessageManager::getInstance();

    testParameterIds (failures);
    testPluginStateRoundTrip (failures);
    testPluginStateRejectsMalformedXml (failures);
    testPresetManagerWorkflow (failures);
    testPresetManagerRestoreSync (failures);
    testOutputSafetyProtector (failures);
    testOutputSafetyProtectorDcBlock (failures);
    testFilterClampsUnsafeCutoffValues (failures);
    testFilterDefaultsAreSafe (failures);
    testAmpEnvelopeHandlesZeroAttackAndRelease (failures);
    testFilterEnvelopeEnableDisableBehavior (failures);
    testVoiceSoundTypePairing (failures);
    testOscillatorVoicePortamentoIsSideEffectFree (failures);
    testSampleVoiceFrequencyTracksRootNote (failures);

    if (failures.isEmpty())
    {
        Logger::writeToLog ("EquinoxCoreTests: all tests passed");
        return 0;
    }

    Logger::writeToLog ("EquinoxCoreTests: failures detected");

    for (const auto& failure : failures)
        Logger::writeToLog (" - " + failure.message);

    return 1;
}
