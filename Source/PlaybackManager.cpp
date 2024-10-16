/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    gainProcessor.setRampDurationSeconds (0.05);
    gainProcessor.setGainDecibels (0.0f);
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
    
    if (isProcessing || isCalibrating)
    {
        juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
        filter.process (ioBlock);
        gainProcessor.process (juce::dsp::ProcessContextReplacing<float> (ioBlock));
    }
}

//void PlaybackManager::updateFilterWithBands (std::vector<Band> bands)
//{
//    filter.setBands (bands, spec.sampleRate);
//}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    gainProcessor.setGainDecibels (bandProfile.getVolume());
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
    gainProcessor.setGainDecibels (volume);
}

void PlaybackManager::startAmplCalibration (float freq, float bandwidth)
{
//    spatialPinkNoiseGenerator.setBandpass (freq, bandwidth);
    
    // Simple Example Pattern
//    MelodicNotes simplePattern =
//    MelodicNotes ({ -12, 0, 12 }, freq)
//        .withBandwidth (bandwidth)
//        .withNoteDurationInSeconds (0.2);
//    MelodicNotes details =
//    MelodicNotes::withMelodicPattern ({ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, { -24, -18, -12, -6, 0, 6, 12, 24, 18, 12, 6, 0, -9, -15 }, freq, 3.0, { 0 })
//        .withCyclingPans ({ -1, -0.5, 0, 0.5, 1, 0.5, 0, -0.5 })
//        .withNoteDurationInSeconds (0.1);
//    spatialPatternGenerator.setPattern (details.noiseNotes());
    
    // JustinPatterns D
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
}

void PlaybackManager::updateAmplCalibration (float freq, float bandwidth)
{
//    spatialPinkNoiseGenerator.setBandpass (freq, bandwidth);
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
