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
    : filter (FFT_SIZE), isCalibrating (false), isBypassed (false), hasPreparedFilter (false)
{}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& buffer)
{
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5;
        }
    }
    else
    {
        // process audio through the filter
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        if (isBypassed && hasPreparedFilter)
        {
            filter.process (context);
            // TODO: Fix this
//            wetGainProcessor.process (context);
        }
        else
        {
//            dryGainProcessor.process (context);
        }
    }
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return sliderSequencer.getNextSample();
}

void PlaybackManager::setSampleRate (float newSampleRate)
{
    sliderSequencer.setSampleRate (newSampleRate);
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsBypassed (bool isBypassed)
{
    this->isBypassed = isBypassed;
}

void PlaybackManager::updateWithCurve (const Curve& curve)
{
    filter.updateWithCurve (curve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    hasPreparedFilter = true;
}
