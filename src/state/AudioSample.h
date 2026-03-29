/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    AudioSample.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/** Immutable description and sample buffer used by the sampler engine. */
struct AudioSample
{
    String sampleName;
    String sampleFormat;
    int samplerate;
    int bitsPerSample;
    int numberOfChannels;
    int lengthInSamples;
    AudioBuffer<float> sampleSource;

    /** Creates an audio sample description from decoded sample metadata and buffer contents. */
    AudioSample (String sampleNameIn,
        String sampleFormatIn,
        int samplerateIn,
        int bitsPerSampleIn,
        int numberOfChannelsIn,
        int lengthInSamplesIn,
        AudioBuffer<float> sampleSourceIn)
        : sampleName (std::move (sampleNameIn)),
          sampleFormat (std::move (sampleFormatIn)),
          samplerate (samplerateIn),
          bitsPerSample (bitsPerSampleIn),
          numberOfChannels (numberOfChannelsIn),
          lengthInSamples (lengthInSamplesIn),
          sampleSource (std::move (sampleSourceIn))
    {
    }
};
