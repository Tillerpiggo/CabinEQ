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
    : filter (FFT_SIZE), isCalibrating (false)
{}

std::pair<float, float> PlaybackManager::getNextSample()
{
    return sliderSequencer.getNextSample();
}

bool PlaybackManager::getIsCalibrating() const
{
    return isCalibrating;
}

void PlaybackManager::setSampleRate (float newSampleRate)
{
    sliderSequencer.setSampleRate (newSampleRate);
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::updateWithCurve (const Curve& curve)
{
    filter.updateWithCurve (curve, FFT_SIZE);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
}
