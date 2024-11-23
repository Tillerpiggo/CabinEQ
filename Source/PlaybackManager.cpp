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
    : firFilter (14),
      tiltFilter (15),
      isFilterOn (true),
      isPlayingNoise (false)
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
    
    if (isPlayingNoise)
    {
        float volumeOffset = juce::Decibels::decibelsToGain (calibrationVolume);
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.15 * 0.5 * volumeOffset;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.15 * 0.5 * volumeOffset;
        }
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext(ioBlock);
    
    // TODO: combine this audio processing logic for compile-time optimization with processorChain
    
    
    if (isFilterOn)
    {
//        firFilter.process (ioBlock);
        if (! isPlayingNoise || filterCycleCounter < 100)
        {
            filter.process (ioBlock);
            profileVolumeProcessor.process (ioContext);
            
            if (isProvisionalOn)
            {
                provisionalFilter.process (ioBlock);
            }
        }
    }
    filterCycleCounter++;
    if (filterCycleCounter > 200)
    {
        filterCycleCounter = 0;
    }
    
    if (isPlayingNoise)
    {
        tiltFilter.process (ioContext);
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
    
    glyphGenerator.prepare (spec);
    fractalPatternGenerator.prepare (spec);
//    pinkNoiseGenerator.setSampleRate (spec.sampleRate);
    filter.prepare (spec);
    provisionalFilter.prepare (spec);
    firFilter.prepare (spec);
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve);
    
    fractalPatternGenerator.setPattern (FractalPattern (4));
}


void PlaybackManager::setIsFilterOn (bool isFilterOn)
{
    this->isFilterOn = isFilterOn;
}

void PlaybackManager::setIsPlayingNoise (bool isPlayingNoise)
{
    this->isPlayingNoise = isPlayingNoise;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    overallVolumeProcessor.setGainDecibels (volume);
}

void PlaybackManager::setSpeedFactor (float speedFactor)
{
    glyphGenerator.setSpeedFactor (speedFactor);
}

void PlaybackManager::setBandwidth (float bandwidth)
{
    glyphGenerator.setBandwidth (bandwidth);
}

void PlaybackManager::setProvisionalBands (std::vector<Band> provisionalBands)
{
    provisionalFilter.setBands (provisionalBands, spec.sampleRate);
}

void PlaybackManager::setProvisionalBandsOn (bool provisionalBandsOn)
{
    this->isProvisionalOn = provisionalBandsOn;
}

void PlaybackManager::setSizeFactor (float sizeFactor)
{
    glyphGenerator.setSizeFactor (sizeFactor);
}

void PlaybackManager::setCenterPos (juce::Point<float> centerPos)
{
    glyphGenerator.setCenterPos (centerPos);
}

void PlaybackManager::setGlyph (Glyph glyph)
{
    glyphGenerator.setGlyph (glyph);
}

float PlaybackManager::getCurrPlayingTime()
{
    return glyphGenerator.getCurrPlayingTime();
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    
    if (clickTrainCounter >= 12000)
    {
        clickTrainCounter = 0;
        return { 1.0f, 1.0f };
    }
    clickTrainCounter++;
    return { 0.0f, 0.0f };
//    return glyphGenerator.getNextSample();
//    float val = pinkNoise.generate();
//    return { val, val };
//    return fractalPatternGenerator.getNextSample();
}
