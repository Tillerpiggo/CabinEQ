/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include "ArbitraryResponseFilter.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false),
      tiltFilter (8),
      arbitrarySequencer (std::make_unique<SineWaveGenerator>()),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator>()),
      arbitrarySequencer3 (std::make_unique<SineWaveGenerator>())
{
    profileVolumeProcessor.setRampDurationSeconds (0.05);
    profileVolumeProcessor.setGainDecibels (0.0f);
    overallVolumeProcessor.setRampDurationSeconds (0.05);
    overallVolumeProcessor.setGainDecibels (0.0f);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.15 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.15 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext(ioBlock);
    
    if (isCalibrating)
    {
        tiltFilter.process (ioContext);
    }
    
    if (isProcessing || isCalibrating)
    {
        filter.process (ioBlock);
        profileVolumeProcessor.process (ioContext);
    }
    
    overallVolumeProcessor.process (ioContext);
}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    profileVolumeProcessor.setGainDecibels (bandProfile.getVolume());
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    filter.prepare (spec);
    spatialPatternGenerator.prepare (spec);
    spatialPatternGenerator2.prepare (spec);
    spatialPatternGenerator3.prepare (spec);
    spatialPatternGenerator4.prepare (spec);
    spatialPatternGenerator5.prepare (spec);
    spatialPatternGenerator6.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    arbitrarySequencer3.setSampleRate (spec.sampleRate);
    spatialPinkNoiseGenerator.setSampleRate (spec.sampleRate);
    wrapperPinkNoiseGenerator.setSampleRate (spec.sampleRate);
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve);
    hasPreparedFilter = true;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    overallVolumeProcessor.setGainDecibels (volume);
}

void PlaybackManager::startAmplCalibration (float freq, float bandwidth)
{
////    // JustinPatterns D
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 400, 1600, 6400, 10000, 8000, 2400, 700, 200 })
//        .withBandwidth (8.0)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -18, -12, -6, 0, 6, 12, 24, 18, 12, 6, 0, -9, -15 }, freq, 3.0, { 0 })
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes sideDrums =
//    MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
//        .withCyclingPans ({ -1, -0.5, 0.5, 1 })
//        .withBandwidth (8.0)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
    
//    // Bands I
//    float semitoneRatio = 5 * bandwidth;
//    MelodicNotes pattern =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, { -semitoneRatio, 0, semitoneRatio, }, freq, bandwidth, { 0 })
//        .withNoteDurationInSeconds (0.1);
//     
//    spatialPatternGenerator.setPattern (pattern.noiseNotes());
    
//    // Bands II
//    float semitoneRatio = 5 * bandwidth;
//    MelodicNotes pattern =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, { -semitoneRatio, 0, semitoneRatio, }, freq, bandwidth / 2.0f, { 0 })
//        .withNoteDurationInSeconds (0.1);
//     
//    spatialPatternGenerator.setPattern (pattern.noiseNotes());
    
//    // Bands III
//    MelodicNotes pattern =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12.0f * bandwidth, -6.0f * bandwidth, 0, 6.0f * bandwidth, 12.0f * bandwidth }, freq, bandwidth / 2.0f, { 0 })
//        .withNoteDurationInSeconds (0.08);
//    
//    // Add noise avoiding the pattern
//    std::vector<float> belowFreqs;
//    std::vector<float> aboveFreqs;
//    float belowFreq = freq / std::pow (2.0f, bandwidth);
//    float aboveFreq = freq * std::pow (2.0f, bandwidth);
//    for (int i = 0; i < 6; ++i)
//    {
//        belowFreqs.push_back (belowFreq);
//        aboveFreqs.push_back (aboveFreq);
//        belowFreq /= 2.0f;
//        aboveFreq *= 2.0f;
//    }
//    
//    MelodicNotes belowNoise =
//    MelodicNotes::withFreqs (belowFreqs)
//        .withBandwidth (3.0f);
//    
//    MelodicNotes aboveNoise =
//    MelodicNotes::withFreqs (aboveFreqs)
//        .withBandwidth (3.0f);
//    
//    spatialPatternGenerator.setPattern (belowNoise.noiseNotes());
//    spatialPatternGenerator2.setPattern (pattern.noiseNotes());
//    spatialPatternGenerator3.setPattern (aboveNoise.noiseNotes());
    
//    // Cabin Noise I
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 400, 1600, 6400, 10000, 8000, 2400, 700, 200 })
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 3.0f, { 0 })
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes sideDrums =
//    MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
//        .withCyclingPans ({ -1, 1 })
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
    
//    // Cabin Noise III
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 36 }, freq)
//        .withBandwidth (2.5)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 3.0f, { 0 })
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes sideDrums =
//    MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
//        .withCyclingPans ({ -1, 1 })
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
    
//    // Cabin Noise III.III - interesting results, not sure how to classify them
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 36, -24, 24 }, freq)
//        .withBandwidths ({ 2.5, 2.5, 1.0, 1.0 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 3.0f, { 0 })
//        .withCyclingPans ({ -1, 1, 1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes sideDrums =
//    MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
//        .withCyclingPans ({ -1, 1 })
//        .withBandwidth (4.0)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
    
//    // Cabin Noise IV
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (2.5)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -18, -18, 18, 18, 18 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (0.5f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    // Cabin Noise V
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, 1.0f, -12.0f);
    
//    // Echoes I
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -1, 0, 0, 1, 0 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 }, { -3, -3, 0, 0, 3, 3, 0, 0 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -1.0, -0.5, 0.0, 0.5, 1.0, 0.5, 0.0, -0.5 })
//        .withCyclingAmpls ({ 0, 0, -6 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    // Surrounded I
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -36, 0, 0, 36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withBandwidth (bandwidth / 3.0f)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
////    MelodicNotes annoyances =
////    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
////        .withBandwidth (bandwidth / 8.0f)
////        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
    
//    // Surrounded II
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -24, 0, 0, 24, 24 }, freq)
//        .withBandwidth (bandwidth * 1.5f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withCyclingAmpls ({ -6 })
//        .withCyclingBandwidths ({ bandwidth / 3.0f, bandwidth / 3.3f, bandwidth / 3.6f, bandwidth / 3.9f })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, -8, -4, 0, 4, 8, 12, 8, 4, 0, -4, -8 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -24, 24 }, freq)
//        .withBandwidth (bandwidth / 1.5f)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
//    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded III
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -36, 0, 0, 36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withCyclingBandwidths ({ bandwidth / 3.0f, bandwidth / 3.3f, bandwidth / 3.6f, bandwidth / 3.9f })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
//    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded IV
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -24, -12, -12, 0, 0, 12, 12, 24, 24 }, freq)
//        .withBandwidth (5.0f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ 0 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (0.5f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -24, 24 }, freq)
//        .withBandwidth (5.0f)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
//    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
    // Surrounded V
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -6, -6, 0, 0, 6, 6 }, freq)
//        .withBandwidth (5.0f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes ({ 0 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (0.5f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -24, 24 }, freq)
//        .withBandwidth (2.0f)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
//    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded II.I
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -24, 0, 0, 24, 24 }, freq)
//        .withBandwidth (bandwidth * 1.5f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withCyclingAmpls ({ -6 })
//        .withCyclingBandwidths ({ bandwidth / 3.0f, bandwidth / 3.3f, bandwidth / 3.6f, bandwidth / 3.9f })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, -8, -4, 0, 4, 8, 12, 8, 4, 0, -4, -8 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -24, 24 }, freq)
//        .withBandwidth (bandwidth / 1.5f)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
////    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded II.II
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -24, 24, 24 }, freq)
//        .withBandwidth (bandwidth * 1.5f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withCyclingAmpls ({ -6 })
//        .withCyclingBandwidths ({ bandwidth / 3.0f, bandwidth / 3.3f, bandwidth / 3.6f, bandwidth / 3.9f })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, -8, -4, 0, 4, 8, 12, 8, 4, 0, -4, -8 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -24, 24 }, freq)
//        .withBandwidth (bandwidth / 1.5f)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
////    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded Experiments
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 0, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
////        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withCyclingBandwidths ({ bandwidth / 3.0f, bandwidth / 3.3f, bandwidth / 3.6f, bandwidth / 3.9f })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
////    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Surrounded Experiments II
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -24, 24, 24 }, freq)
//        .withBandwidth (6.0f)
//        .withCyclingPans ({ -1, 1 })
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -0.5, -0.4, -0.3, -0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0, -0.1, -0.2, -0.3, -0.4 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 6.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    MelodicNotes topDrums =
//    MelodicNotes ({ -36, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingPans ({ -0.8, -0.8, -0.4, -0.4, 0.0, 0.0, 0.4, 0.4, 0.8, 0.8 })
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (topDrums.noiseNotes());
////    spatialPatternGenerator4.setPattern (annoyances.noiseNotes());
    
//    // Dips I
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 0 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth / 2.0f, -12.0f);
    
//    // Dips II
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 0 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -12.0f);
    
//    // Dips III
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -12, 12 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -12.0f);
    
//    // Dips IV
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -12, 0, 12, 0 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 4.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -8.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -8.0f);
    
//    // Dips V
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -12, 0, 12, 0 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 4.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -4.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -4.0f);
    
//    // Dips VI
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -18, -6, 6, 18, 6, -6 }, freq)
//        .withBandwidth (bandwidth * 3.0f)
//        .withCyclingPans (8)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 4.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -10.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -10.0f);
    
//    // Dips VII
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth * 3.0f)
//        .withCyclingPans (20)
//        .withNoteDurationInSeconds (0.05);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, -6, 0, 6, 12, 6, 0, -6 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 4.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -12.0f);
    
//    // Dips VIII
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth * 3.0f)
//        .withCyclingPans (20)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 2.0f, { 0 })
//        .withCyclingPans (20)
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -12, -6, 0, 6, 12, 6, 0, -6 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth / 4.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -24.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -24.0f);
    
//    // Cabin Noise V (2)
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -4, -2, 0, 2, 4, 1, -3 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq, bandwidth, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -12.0f);
    
//    // Separations
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ 0 }, freq)
//        .withCyclingPans (10)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { 12 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
//    
//    spatialPatternGenerator.setPeakFilter (freq * 2.0f, bandwidth, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, bandwidth, -12.0f);
    
//    // Cabin Noise VI
//    float octDiff = bandwidth;
//    float ratio = octDiff / 2.0f;
//    
//    float bandwidthAdjusted = 1.5 * ratio;
//    float semitonesAbove = ratio * 18.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.2);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    spatialPatternGenerator.setPeakFilter (freq, 1.0f, -12.0f);
//    spatialPatternGenerator3.setPeakFilter (freq, 1.0f, -12.0f);
    
//    // Cabin Noise VII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 0, 36, 0 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, 0.5, 0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    // Cabin Noise VIII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, 18, -18, 0, 36 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 4.0f, { 0 })
//        .withCyclingPans ({ -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    // Cabin Noise IX
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 4.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (annoyances.noiseNotes());
    
//    // Cabin Noise X
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
//        .withBandwidth (bandwidth)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -6 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XI
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
//        .withBandwidth (bandwidth / 3.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
//        .withBandwidth (bandwidth / 3.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -6 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XIII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XIV - best so far, more open and natural but still deep and resolving
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24, 36, 48 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15, 21, 27 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, -6, 0, 6, 12, 4, -4 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24, 36, 48 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15, 21, 27 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -12, 0, 12, 24, 8, -8 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XVI
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (bandwidth * 3.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -21, -15, -9, -3, 3, 9, 15, 21 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -12, 0, 12, 24, 8, -8 }, freq, bandwidth / 8.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XVII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (bandwidth * 3.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -21, -15, -9, -3, 3, 9, 15, 21 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0 }, { -12, -12, 0, 0, 12, 12, 24, 24 }, freq, bandwidth / 8.0f, { 0 })
//        .withCyclingAmpls ({ 12, 12, 0, 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Cabin Noise XVIII
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24, 36, 48 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -24 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15, 21, 27 }, freq)
//        .withBandwidth (bandwidth / 2.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -12, 0, 12, 24, 8, -8 }, freq, bandwidth / 16.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withCyclingAmpls ({ 12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24, 36, 48 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15, 21, 27 }, freq)
//        .withBandwidth (bandwidth / 4.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 2, 4, 5, 7, 9, 11, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 5;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales II
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -24, -12, 0, 12, 24, 36, 48 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15, 21, 27 }, freq)
//        .withBandwidth (bandwidth / 4.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { -24, -20, -17, -12, -8, -5, 0, 4, 7, 12, 16, 19, 24 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 5;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales III
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (bandwidth / 4.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { -12, -8, -5, 0, 4, 7, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 5;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales IV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (bandwidth * 2.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (bandwidth / 4.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 4, 0, 7, 0, 14, 12 };
////    for (int i = 0; i < scale.size(); ++i)
////        scale[i] -= 5;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales V
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (1.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 4, 0, 7, 0, 14, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 3;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Scales VI
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.1f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 4, 0, 7, 0, 14, 12, 14, 12, 9, 5, 3, 12, 9, 5, 3 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 5;
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Sandwich I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -6, 6 }, freq)
//        .withBandwidth (bandwidth / 3.0f)
////        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -15, -9, -3, 3, 9, 15 }, freq)
//        .withBandwidth (bandwidth / 4.0f)
////        .withCyclingPans ({ -0.8, 0.8 })
////        .withCyclingAmpls ({ -6 })
//        .withNoteDurationInSeconds (0.1);
//     
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 0, 1, 0, 0, 1, 0 }, { -12, 0, 12 }, freq, bandwidth / 3.0f, { 0 })
////        .withCyclingPans ({ -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1 })
////        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
////        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -6, -3, 0, 3, 6, 3, 0, -3 }, freq)
//        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (bandwidth / 8.0f)
//        .withNoteDurationInSeconds (0.05);
//     
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (details.noiseNotes());
////    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Chords I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.1f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 2, 4, 5, 7, 9, 11, 12, 11, 9, 7, 5, 4, 2 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 5;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    arbitrarySequencer2.setNotes (scale2, freq);
//    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.1f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, -1, -8, -5, -3, -3, 4, -3, -1, -1, 0, -1, -8, -5, -3, -8 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] += 3;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky I
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.1f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, -1, -8, -5, -3, -3, 4, -3, -1, -1, 0, -1, -8, -5, -3, -8 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] += 3;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky II
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (4.0f)
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.1f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] += 3;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky III
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes ({ -36, -24, -12, 0, 12, 24, 36 }, freq)
//        .withBandwidth (8.0f)
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ -24, -15, -9, -3, 3, 9, 15, 24 }, freq)
//        .withBandwidth (0.5f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky IV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 300, 600, 1200, 2400, 4800, 9600 })
//        .withBandwidths ({ 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f })
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ 1000, 2000, 3000, 4000, 5000, 6000, 7000 }, freq)
//        .withBandwidth (0.5f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    arbitrarySequencer2.setNotes (scale2, freq);
//    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Coverage I
//    bandwidth *= 2.0f;
//     
////    MelodicNotes drums =
////    MelodicNotes::withFreqs ({ 10000, 5000, 7500, 5000 })
////        .withBandwidth (0.5f)
////        .withCyclingAmpls ({ -12 })
////        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 500, 700, 1000, 1400, 2000, 1400, 1000 })
//        .withBandwidth (0.05f)
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ 1000, 2000, 3000, 4000, 5000, 6000, 7000 }, freq)
//        .withBandwidth (0.5f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
////        .withCyclingPans ({ -1, 0, 1 })
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
////    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Sines I
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    std::vector<float> confoundingNotes;
//    for (int i = -12; i < 12; i += 2)
//        confoundingNotes.push_back (i);
//    
//    for (int i = 12; i > -12; i -= 2)
//        confoundingNotes.push_back (i);
//    
//    
//    arbitrarySequencer.setNotes (scale, freq);
//    arbitrarySequencer2.setNotes (confoundingNotes, freq, 1500);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
////    spatialPatternGenerator.setPattern (drums.noiseNotes());
//////    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
////    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
//    // Tricky V
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 300, 600, 1200, 2400, 4800, 9600 })
//        .withBandwidth (3.0f)
//        .withCyclingAmpls ({ -12 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes thinDrums =
//    MelodicNotes ({ 1000, 2000, 3000, 4000, 5000, 6000, 7000 }, freq)
//        .withBandwidth (0.5f)
////        .withCyclingPans ({ -0.8, 0.8 })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes annoyances =
//    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
//        .withCyclingPans (9)
//        .withBandwidth (1.0f)
//        .withNoteDurationInSeconds (0.05);
//    
//    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
//    for (int i = 0; i < scale.size(); ++i)
//        scale[i] -= 12;
//    
//    std::vector<float> scale2;
//    std::vector<float> scale3;
//    
//    for (const auto& note : scale)
//    {
//        scale2.push_back (note + 12);
//        scale3.push_back (note - 12);
//    }
//    
//    arbitrarySequencer.setNotes (scale, freq);
////    arbitrarySequencer2.setNotes (scale2, freq);
////    arbitrarySequencer3.setNotes (scale3, freq);
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
//    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
    
    // Tricky VI
    bandwidth *= 2.0f;
     
    MelodicNotes drums =
    MelodicNotes::withFreqs ({ 300, 600, 1200, 2400, 4800, 9600 })
        .withBandwidth (3.0f)
        .withCyclingAmpls ({ -12 })
        .withNoteDurationInSeconds (0.1);
    
    MelodicNotes thinDrums =
    MelodicNotes ({ 1000, 2000, 3000, 4000, 5000, 6000, 7000 }, freq)
        .withBandwidth (0.5f)
//        .withCyclingPans ({ -0.8, 0.8 })
        .withCyclingAmpls ({ 0 })
        .withNoteDurationInSeconds (0.1);
    
    MelodicNotes annoyances =
    MelodicNotes ({ -18, -12, -6, 0, 6, 12, 18 }, freq)
        .withCyclingPans (9)
        .withBandwidth (1.0f)
        .withNoteDurationInSeconds (0.05);
    
    std::vector<float> scale { 0, 12, 24, 12, 7, 19, 21, 15, 4, 16, 17, 16, 17, 16, 12, 12 };
    for (int i = 0; i < scale.size(); ++i)
        scale[i] -= 12;
    
    std::vector<float> scale2;
    std::vector<float> scale3;
    
    for (const auto& note : scale)
    {
        scale2.push_back (note + 12);
        scale3.push_back (note - 12);
    }
    
    arbitrarySequencer.setNotes (scale, freq);
//    arbitrarySequencer2.setNotes (scale2, freq);
//    arbitrarySequencer3.setNotes (scale3, freq);
    spatialPatternGenerator.setPattern (drums.noiseNotes());
    spatialPatternGenerator2.setPattern (annoyances.noiseNotes());
    spatialPatternGenerator3.setPattern (thinDrums.noiseNotes());
}

void PlaybackManager::updateAmplCalibration (float freq, float bandwidth)
{
     
}

void PlaybackManager::setPatternSolo (bool solo)
{
    if (solo)
    {
        isSpatialPatternGeneratorMuted = true;
        isSpatialPatternGenerator2Muted = false;
        isSpatialPatternGenerator3Muted = true;
        isSpatialPatternGenerator4Muted = true;
    }
    else
    {
        isSpatialPatternGeneratorMuted = false;
        isSpatialPatternGenerator2Muted = false;
        isSpatialPatternGenerator3Muted = false;
        isSpatialPatternGenerator4Muted = false;
    }
}

void PlaybackManager::setMutedGenerators (std::vector<bool> mutedGens)
{
    isSpatialPatternGeneratorMuted = mutedGens[0];
    isSpatialPatternGenerator2Muted = mutedGens[1];
    isSpatialPatternGenerator3Muted = mutedGens[2];
    isSpatialPatternGenerator4Muted = mutedGens[3];
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

std::pair<float, float> PlaybackManager::getNextSample()
{
//    return spatialPatternGenerator.getNextSample();
//    return spatialPinkNoiseGenerator.getNextSample();
//    return wrapperPinkNoiseGenerator.getNextSample();
    
    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
    auto [leftSample3, rightSample3] = spatialPatternGenerator3.getNextSample();
    auto [leftSample4, rightSample4] = spatialPatternGenerator4.getNextSample();
    auto [leftSample5, rightSample5] = arbitrarySequencer.getNextSample();
    auto [leftSample6, rightSample6] = arbitrarySequencer2.getNextSample();
    auto [leftSample7, rightSample7] = arbitrarySequencer3.getNextSample();
    
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    
    if (! isSpatialPatternGeneratorMuted)
    {
        leftSample += leftSample1;
        rightSample += rightSample1;
    }
    
    if (! isSpatialPatternGenerator2Muted)
    {
        leftSample += leftSample2;
        rightSample += rightSample2;
    }
    
    if (! isSpatialPatternGenerator3Muted)
    {
        leftSample += leftSample3;
        rightSample += rightSample3;
    }
    
    if (! isSpatialPatternGenerator4Muted)
    {
        leftSample += leftSample4;
        rightSample += rightSample4;
    }
    
    leftSample += leftSample5 * 0.01;
    rightSample += rightSample5 * 0.01;
    
    leftSample += leftSample6 * 0.008;
    rightSample += rightSample6 * 0.008;
    
//    leftSample += leftSample7 * 0.01;
//    rightSample += rightSample7 * 0.01;
    
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample * 30.0, rightSample * 30.0 };
}
