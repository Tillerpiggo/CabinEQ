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
    : isFilterOn (true),
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
        filter.process (ioBlock);
        profileVolumeProcessor.process (ioContext);
        
        if (isProvisionalOn)
        {
            provisionalFilter.process (ioBlock);
        }
    }
    
//    if (isPlayingNoise)
//    {
//        tiltFilter.process (ioContext);
//    }
    
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
    
    gridSequencer.prepare (spec);
    filter.prepare (spec);
    provisionalFilter.prepare (spec);
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
    // TODO: implement
}

void PlaybackManager::setBandwidth (float bandwidth)
{
    // TODO: implement
}

void PlaybackManager::setProvisionalBands (std::vector<Band> provisionalBands)
{
    provisionalFilter.setBands (provisionalBands, spec.sampleRate);
}

void PlaybackManager::setProvisionalBandsOn (bool provisionalBandsOn)
{
    this->isProvisionalOn = provisionalBandsOn;
}

void PlaybackManager::setGrid (NoiseSequenceGrid grid)
{
    gridSequencer.setNoiseGrid (grid);
}

float PlaybackManager::getCurrPlayingTime()
{
    // TODO: implement, or decide to implement this a different way
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return gridSequencer.getNextSample();
}
