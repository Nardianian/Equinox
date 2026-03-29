/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AudioSampleValueTreeState.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once
#include "AudioSample.h"
#include <JuceHeader.h>

/** Stores serialized audio samples and maintains a prepared playback cache for the sampler engine. */
class AudioSampleValueTreeState
{
public:
    using AudioSamplePtr = std::shared_ptr<const AudioSample>;

    /** Creates a sample state store with the supplied ValueTree type. */
    AudioSampleValueTreeState (const Identifier& type);

    /** Stops the cache builder thread and releases cached sample data. */
    ~AudioSampleValueTreeState();

    /** Adds a file as an AudioSample to the AudioSampleValueTreeState*/
    void addAudioSample (const File& audioFile, const String& sampleId);

    /** Returns a pointer to a specific sample*/
    std::unique_ptr<AudioSample> getAudioSample (const String& sampleId) const;

    /** Returns a cached sample prepared for the current playback sample rate. */
    AudioSamplePtr getPreparedAudioSample (const String& sampleId) const;

    /** Replaces the internal ValueTree and schedules a cache rebuild. */
    void replaceState (ValueTree newState);

    /** Updates the playback sample rate used to prepare cached samples. */
    void setPlaybackSampleRate (double newPlaybackSampleRate);

    /** Returns the current serialized sample-state version. */
    int getStateVersion() const noexcept;
    /** Returns the version of the most recently prepared sample cache. */
    int getPreparedStateVersion() const noexcept;

    ValueTree state;

    AudioFormatManager formatManager;

private:
    /** Cached decoded and playback-rate-adjusted sample data for one sample ID. */
    struct CachedSample
    {
        String sampleId;
        AudioSamplePtr decodedSample;
        AudioSamplePtr preparedSample;
    };

    static AudioBuffer<float> readSampleSource (AudioFormatManager& formatManager,
        const ValueTree& audioSampleAsValueTree);
    static AudioBuffer<float>
        resampleBuffer (const AudioBuffer<float>& sourceBuffer, double sourceSampleRate, double targetSampleRate);

    Array<CachedSample> createCachedSamples (const ValueTree& stateSnapshot, double targetPlaybackSampleRate);
    AudioSamplePtr createDecodedSample (const ValueTree& audioSampleAsValueTree);
    AudioSamplePtr createPreparedSample (AudioSamplePtr decodedSample, double targetPlaybackSampleRate) const;
    const CachedSample* findCachedSample (const String& sampleId) const;
    void scheduleCacheRebuild();
    bool getPendingCacheBuild (ValueTree& stateSnapshot, double& targetPlaybackSampleRate, int& version);
    void applyPreparedCache (Array<CachedSample> preparedCache, int version);
    void cacheBuilderLoop();

    CriticalSection stateLock;
    CriticalSection formatManagerLock;
    WaitableEvent cacheBuildRequested;
    std::thread cacheBuilderThread;
    std::atomic<bool> stopCacheBuilder { false };
    ValueTree pendingStateSnapshot;
    double pendingPlaybackSampleRate = 0.0;
    int pendingCacheBuildVersion = 0;
    bool hasPendingCacheBuild = false;
    double playbackSampleRate = 0.0;
    Array<CachedSample> cachedSamples;
    std::atomic<int> stateVersion { 0 };
    std::atomic<int> preparedStateVersion { 0 };
};
