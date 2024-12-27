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
      tiltFilter (12),
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
            leftChannel[sample] += value.first * 0.15 * 0.5 * volumeOffset;
            
            if (rightChannel)
                rightChannel[sample] += value.second * 0.15 * 0.5 * volumeOffset;
        }
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext (ioBlock);
    
    if (isFilterOn)
    {
        if (isIIR)
        {
            filter.process (ioBlock);
        }
        else
        {
            firFilter.process (ioContext);
        }
        
        profileVolumeProcessor.process (ioContext);
        
        if (isProvisionalOn)
        {
            provisionalFilter.process (ioBlock);
        }
    }
    
    if (isPlayingNoise)
    {
        tiltFilter.process (ioContext);
    }
    
    overallVolumeProcessor.process (ioContext);
}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    this->bandEqCurve.updateWithBands (bandProfile.getBands());
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    profileVolumeProcessor.setGainDecibels (bandProfile.getVolume());
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    
    glyphGridPlayer.prepare (spec);
    gridSequencer.prepare (spec);
    filter.prepare (spec);
    provisionalFilter.prepare (spec);
    firFilter.prepare (spec);
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve, 12);
//    firFilter.updateWithCurve (firCurve);
}


void PlaybackManager::setIsFilterOn (bool isFilterOn)
{
    this->isFilterOn = isFilterOn;
}

void PlaybackManager::setIsPlayingNoise (bool isPlayingNoise)
{
    this->isPlayingNoise = isPlayingNoise;
}

void PlaybackManager::setIsCabinNoise (bool isCabinNoise)
{
    this->isCabinNoise = isCabinNoise;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    overallVolumeProcessor.setGainDecibels (volume);
}

void PlaybackManager::setMinFreq (float minFreq)
{
    glyphGridPlayer.setMinFreq (minFreq);
    gridSequencer.setMinFreq (minFreq);
}

void PlaybackManager::setMaxFreq (float maxFreq)
{
    glyphGridPlayer.setMaxFreq (maxFreq);
    gridSequencer.setMaxFreq (maxFreq);
}

void PlaybackManager::setSpeedFactor (float speedFactor)
{
    glyphGridPlayer.setSpeedFactor (speedFactor);
    gridSequencer.setSpeedFactor (speedFactor);
}

void PlaybackManager::setBandwidth (float bandwidth)
{
    glyphGridPlayer.setBandwidth (bandwidth);
    gridSequencer.setBandwidth (bandwidth);
}

void PlaybackManager::updateFIRFilter()
{
    // Calculate curve pts
    std::vector<CurvePt> leftCurvePts;
    std::vector<CurvePt> rightCurvePts;
    const float startFreq = 20.0f;
    const float endFreq = 20000.0f;
    const int numPoints = 4000;
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = startFreq * std::pow (endFreq / startFreq, i / (numPoints - 1.0f));
        float leftAmpl = bandEqCurve.leftDbAtFrequency (freq);
        float rightAmpl = bandEqCurve.rightDbAtFrequency (freq);
        leftCurvePts.push_back (CurvePt (i, freq, leftAmpl));
        rightCurvePts.push_back (CurvePt (i, freq, rightAmpl));
    }
    
    firCurve.updateWithCurvePts (leftCurvePts, rightCurvePts);
    firFilter.updateWithCurve (firCurve, fftSize);
}

void PlaybackManager::setFIRQuality (int fftSize)
{
    this->fftSize = fftSize;
}

void PlaybackManager::setIIR (bool isIIR)
{
    this->isIIR = isIIR;
}

void PlaybackManager::setProvisionalBands (std::vector<Band> provisionalBands)
{
    provisionalFilter.setBands (provisionalBands, spec.sampleRate);
}

void PlaybackManager::setProvisionalBandsOn (bool provisionalBandsOn)
{
    this->isProvisionalOn = provisionalBandsOn;
}

void PlaybackManager::setGlyphs (std::vector<Glyph> glyphs)
{
    glyphGridPlayer.setGlyphs (glyphs);
}

void PlaybackManager::setGrid (NoiseSequenceGrid grid)
{
    gridSequencer.setNoiseGrid (grid);
}

float PlaybackManager::getCurrPlayingTime()
{
//    return gridSequencer.getCurrTime();
    return glyphGridPlayer.getCurrPlayingTime();
}

std::vector<float> PlaybackManager::getCurrPlayingFreqs()
{
    return gridSequencer.getCurrPlayingFreqs();
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return glyphGridPlayer.getNextSample();
//    return gridSequencer.getNextSample();
}
