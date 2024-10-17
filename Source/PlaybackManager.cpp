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
      tiltFilter (8)
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
    spatialPatternGenerator.setSampleRate (spec.sampleRate);
    spatialPatternGenerator2.setSampleRate (spec.sampleRate);
    spatialPatternGenerator3.setSampleRate (spec.sampleRate);
    spatialPatternGenerator4.setSampleRate (spec.sampleRate);
    spatialPatternGenerator5.setSampleRate (spec.sampleRate);
    spatialPatternGenerator6.setSampleRate (spec.sampleRate);
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
//    // JustinPatterns D
    float octDiff = bandwidth;
    float ratio = octDiff / 2.0f;
    
    float bandwidthAdjusted = 1.5 * ratio;
    float semitonesAbove = ratio * 18.0f;
     
    MelodicNotes drums =
    MelodicNotes::withFreqs ({ 100, 400, 1600, 6400, 10000, 8000, 2400, 700, 200 })
        .withBandwidth (8.0)
        .withNoteDurationInSeconds (0.1);
     
    MelodicNotes details =
    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -18, -12, -6, 0, 6, 12, 24, 18, 12, 6, 0, -9, -15 }, freq, 3.0, { 0 })
        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
        .withNoteDurationInSeconds (0.1);
    
    
    MelodicNotes sideDrums =
    MelodicNotes::withFreqs ({ 100, 100, 400, 400, 1600, 1600, 6400, 6400, 10000, 10000, 8000, 8000, 2400, 2400, 700, 700, 200, 200 })
        .withCyclingPans ({ -1, -0.5, 0.5, 1 })
        .withBandwidth (8.0)
        .withNoteDurationInSeconds (0.05);
     
    spatialPatternGenerator.setPattern (drums.noiseNotes());
    spatialPatternGenerator2.setPattern (details.noiseNotes());
    spatialPatternGenerator3.setPattern (sideDrums.noiseNotes());
    
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
    
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    
    if (! isSpatialPatternGeneratorMuted)
    {
        leftSample += leftSample1;
        rightSample += rightSample1;
    }
    
    if (! isSpatialPatternGenerator2Muted)
    {
        leftSample += leftSample2 * 2.0;
        rightSample += rightSample2 * 2.0;
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
    
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample * 3.0, rightSample * 3.0 };
}
