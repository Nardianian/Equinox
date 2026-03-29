/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AudioSampleValueTreeState.cpp
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#include "AudioSampleValueTreeState.h"

AudioSampleValueTreeState::AudioSampleValueTreeState (const Identifier& type)
{
    state = ValueTree (type);
    formatManager.registerBasicFormats();
    cacheBuilderThread = std::thread ([this] { cacheBuilderLoop(); });
    scheduleCacheRebuild();
}

AudioSampleValueTreeState::~AudioSampleValueTreeState()
{
    stopCacheBuilder.store (true);
    cacheBuildRequested.signal();

    if (cacheBuilderThread.joinable())
        cacheBuilderThread.join();
}

void AudioSampleValueTreeState::addAudioSample (const File& audioFile, const String& sampleId)
{
    std::unique_ptr<AudioFormatReader> formatReader;

    {
        const ScopedLock formatLock (formatManagerLock);
        formatReader.reset (formatManager.createReaderFor (audioFile));
    }

    if (formatReader == nullptr)
        return;

    const auto fileExtension = formatReader->getFormatName().dropLastCharacters (5);
    MemoryBlock sampleSourceBlock;

    {
        const ScopedLock formatLock (formatManagerLock);
        auto* format = formatManager.findFormatForFileExtension (fileExtension);

        if (format == nullptr)
            return;

        std::unique_ptr<OutputStream> outputStream = std::make_unique<MemoryOutputStream> (sampleSourceBlock, false);
        auto writerOptions = AudioFormatWriterOptions {}
                                 .withSampleRate (formatReader->sampleRate)
                                 .withNumChannels (static_cast<int> (formatReader->numChannels))
                                 .withBitsPerSample (static_cast<int> (formatReader->bitsPerSample));
        auto writer = format->createWriterFor (outputStream, writerOptions);

        if (writer == nullptr)
            return;

        writer->writeFromAudioReader (*formatReader, 0, formatReader->lengthInSamples);
    }

    ValueTree audioSample (sampleId);
    audioSample.setProperty ("samplename", var (audioFile.getFileName()), nullptr);
    audioSample.setProperty ("sampleformat", var (fileExtension), nullptr);
    audioSample.setProperty ("samplerate", var (formatReader->sampleRate), nullptr);
    audioSample.setProperty ("bitspersample", var (static_cast<int> (formatReader->bitsPerSample)), nullptr);
    audioSample.setProperty ("numberofchannels", var (static_cast<int> (formatReader->numChannels)), nullptr);
    audioSample.setProperty ("lengthinsamples", var (formatReader->lengthInSamples), nullptr);
    audioSample.setProperty ("samplesource", var (sampleSourceBlock), nullptr);

    {
        const ScopedLock lock (stateLock);

        if (auto existingSample = state.getChildWithName (sampleId); existingSample.isValid())
            state.removeChild (existingSample, nullptr);

        state.addChild (audioSample, state.getNumChildren(), nullptr);
        ++stateVersion;
    }

    scheduleCacheRebuild();
}

std::unique_ptr<AudioSample> AudioSampleValueTreeState::getAudioSample (const String& sampleId) const
{
    if (auto sample = getPreparedAudioSample (sampleId))
        return std::make_unique<AudioSample> (*sample);

    return nullptr;
}

AudioSampleValueTreeState::AudioSamplePtr AudioSampleValueTreeState::getPreparedAudioSample (
    const String& sampleId) const
{
    const ScopedLock lock (stateLock);

    if (const auto* cachedSample = findCachedSample (sampleId))
        return cachedSample->preparedSample != nullptr ? cachedSample->preparedSample : cachedSample->decodedSample;

    return nullptr;
}

void AudioSampleValueTreeState::replaceState (ValueTree newState)
{
    {
        const ScopedLock lock (stateLock);
        state = newState;
        ++stateVersion;
    }

    scheduleCacheRebuild();
}

void AudioSampleValueTreeState::setPlaybackSampleRate (double newPlaybackSampleRate)
{
    if (newPlaybackSampleRate <= 0.0)
        return;

    bool sampleRateChanged = false;

    {
        const ScopedLock lock (stateLock);

        if (!approximatelyEqual (playbackSampleRate, newPlaybackSampleRate))
        {
            playbackSampleRate = newPlaybackSampleRate;
            ++stateVersion;
            sampleRateChanged = true;
        }
    }

    if (sampleRateChanged)
        scheduleCacheRebuild();
}

int AudioSampleValueTreeState::getStateVersion() const noexcept { return stateVersion.load(); }

int AudioSampleValueTreeState::getPreparedStateVersion() const noexcept { return preparedStateVersion.load(); }

AudioBuffer<float> AudioSampleValueTreeState::readSampleSource (AudioFormatManager& formatManager,
    const ValueTree& audioSampleAsValueTree)
{
    const auto channelCount = static_cast<int> (audioSampleAsValueTree.getProperty ("numberofchannels"));
    const auto lengthInSamples = static_cast<int> (audioSampleAsValueTree.getProperty ("lengthinsamples"));
    AudioBuffer<float> sampleSource (channelCount, lengthInSamples);

    if (auto* sampleSourceBlock = audioSampleAsValueTree.getProperty ("samplesource").getBinaryData())
    {
        if (auto* format = formatManager.findFormatForFileExtension (
                audioSampleAsValueTree.getProperty ("sampleformat").toString()))
        {
            std::unique_ptr<AudioFormatReader> reader (
                format->createReaderFor (new MemoryInputStream (*sampleSourceBlock, false), true));

            if (reader != nullptr)
                reader->read (&sampleSource, 0, lengthInSamples, 0, true, true);
        }
    }

    return sampleSource;
}

AudioBuffer<float> AudioSampleValueTreeState::resampleBuffer (const AudioBuffer<float>& sourceBuffer,
    double sourceSampleRate,
    double targetSampleRate)
{
    if (sourceSampleRate <= 0.0 || targetSampleRate <= 0.0 || approximatelyEqual (sourceSampleRate, targetSampleRate))
        return AudioBuffer<float> (sourceBuffer);

    const auto speedRatio = sourceSampleRate / targetSampleRate;
    const auto targetLength = jmax (1, roundToInt (sourceBuffer.getNumSamples() / speedRatio));
    AudioBuffer<float> resampledBuffer (sourceBuffer.getNumChannels(), targetLength);

    for (int channel = 0; channel < sourceBuffer.getNumChannels(); ++channel)
    {
        CatmullRomInterpolator interpolator;
        interpolator.reset();
        interpolator.process (
            speedRatio, sourceBuffer.getReadPointer (channel), resampledBuffer.getWritePointer (channel), targetLength);
    }

    return resampledBuffer;
}

Array<AudioSampleValueTreeState::CachedSample>
    AudioSampleValueTreeState::createCachedSamples (const ValueTree& stateSnapshot, double targetPlaybackSampleRate)
{
    Array<CachedSample> preparedCache;

    for (int index = 0; index < stateSnapshot.getNumChildren(); ++index)
    {
        auto audioSampleAsValueTree = stateSnapshot.getChild (index);
        auto decodedSample = createDecodedSample (audioSampleAsValueTree);

        CachedSample cachedSample;
        cachedSample.sampleId = audioSampleAsValueTree.getType().toString();
        cachedSample.decodedSample = decodedSample;
        cachedSample.preparedSample = createPreparedSample (decodedSample, targetPlaybackSampleRate);
        preparedCache.add (std::move (cachedSample));
    }

    return preparedCache;
}

AudioSampleValueTreeState::AudioSamplePtr AudioSampleValueTreeState::createDecodedSample (
    const ValueTree& audioSampleAsValueTree)
{
    if (!audioSampleAsValueTree.isValid())
        return nullptr;

    AudioBuffer<float> sampleSource;

    {
        const ScopedLock formatLock (formatManagerLock);
        sampleSource = readSampleSource (formatManager, audioSampleAsValueTree);
    }

    return std::make_shared<AudioSample> (audioSampleAsValueTree.getProperty ("samplename").toString(),
        audioSampleAsValueTree.getProperty ("sampleformat").toString(),
        static_cast<int> (audioSampleAsValueTree.getProperty ("samplerate")),
        static_cast<int> (audioSampleAsValueTree.getProperty ("bitspersample")),
        static_cast<int> (audioSampleAsValueTree.getProperty ("numberofchannels")),
        sampleSource.getNumSamples(),
        std::move (sampleSource));
}

AudioSampleValueTreeState::AudioSamplePtr AudioSampleValueTreeState::createPreparedSample (AudioSamplePtr decodedSample,
    double targetPlaybackSampleRate) const
{
    if (decodedSample == nullptr)
        return nullptr;

    if (targetPlaybackSampleRate <= 0.0
        || approximatelyEqual (static_cast<double> (decodedSample->samplerate), targetPlaybackSampleRate))
    {
        return decodedSample;
    }

    auto preparedSource =
        resampleBuffer (decodedSample->sampleSource, decodedSample->samplerate, targetPlaybackSampleRate);

    return std::make_shared<AudioSample> (decodedSample->sampleName,
        decodedSample->sampleFormat,
        roundToInt (targetPlaybackSampleRate),
        decodedSample->bitsPerSample,
        preparedSource.getNumChannels(),
        preparedSource.getNumSamples(),
        std::move (preparedSource));
}

const AudioSampleValueTreeState::CachedSample* AudioSampleValueTreeState::findCachedSample (
    const String& sampleId) const
{
    for (const auto& cachedSample : cachedSamples)
        if (cachedSample.sampleId == sampleId)
            return &cachedSample;

    return nullptr;
}

void AudioSampleValueTreeState::scheduleCacheRebuild()
{
    {
        const ScopedLock lock (stateLock);
        pendingStateSnapshot = state.createCopy();
        pendingPlaybackSampleRate = playbackSampleRate;
        pendingCacheBuildVersion = stateVersion.load();
        hasPendingCacheBuild = true;
    }

    cacheBuildRequested.signal();
}

bool AudioSampleValueTreeState::getPendingCacheBuild (ValueTree& stateSnapshot,
    double& targetPlaybackSampleRate,
    int& version)
{
    const ScopedLock lock (stateLock);

    if (!hasPendingCacheBuild)
        return false;

    stateSnapshot = pendingStateSnapshot;
    targetPlaybackSampleRate = pendingPlaybackSampleRate;
    version = pendingCacheBuildVersion;
    hasPendingCacheBuild = false;
    return true;
}

void AudioSampleValueTreeState::applyPreparedCache (Array<CachedSample> preparedCache, int version)
{
    const ScopedLock lock (stateLock);

    if (version != stateVersion.load())
        return;

    cachedSamples = std::move (preparedCache);
    preparedStateVersion.store (version);
}

void AudioSampleValueTreeState::cacheBuilderLoop()
{
    while (!stopCacheBuilder.load())
    {
        cacheBuildRequested.wait (-1);

        if (stopCacheBuilder.load())
            return;

        while (!stopCacheBuilder.load())
        {
            ValueTree stateSnapshot;
            double targetPlaybackSampleRate = 0.0;
            int version = 0;

            if (!getPendingCacheBuild (stateSnapshot, targetPlaybackSampleRate, version))
                break;

            applyPreparedCache (createCachedSamples (stateSnapshot, targetPlaybackSampleRate), version);
        }
    }
}
