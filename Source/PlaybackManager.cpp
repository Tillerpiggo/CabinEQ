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
    sineSweepGenerator.setSampleRate (spec.sampleRate);
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
//    // Tricky IV
//    bandwidth *= 2.0f;
//     
//    MelodicNotes drums =
//    MelodicNotes::withFreqs ({ 100, 300, 900, 2700, 8100 })
//        .withBandwidths ({ 1.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes drums2 =
//    MelodicNotes::withFreqs ({ 500, 1500, 5000 })
//        .withBandwidths ({ 4.0f })
//        .withCyclingAmpls ({ 0 })
//        .withNoteDurationInSeconds (0.1);
//    
//    MelodicNotes backgroundNoise =
//    MelodicNotes::withFreqs ({ 1000 })
//        .withBandwidth (10.0f)
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
//    spatialPatternGenerator.setPattern (drums.noiseNotes());
//    spatialPatternGenerator2.setPattern (backgroundNoise.noiseNotes());
//    spatialPatternGenerator3.setPattern (drums2.noiseNotes());
    
    sineSweepGenerator.setSweepPattern (SweepPattern (freq, bandwidth, 1.0f, spec.sampleRate));
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

float PlaybackManager::getCurrPlayingFreq()
{
    return sineSweepGenerator.getCurrFreq();
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return sineSweepGenerator.getNextSample();
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
    
//    leftSample += leftSample6 * 0.1;
//    rightSample += rightSample6 * 0.1;
    
//    leftSample += leftSample7 * 0.01;
//    rightSample += rightSample7 * 0.01;
    
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample * 30.0, rightSample * 30.0 };
}
