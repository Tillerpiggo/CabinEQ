/*
  ==============================================================================

    RandomCalibrationManager.cpp
    Created: 23 Jun 2024 6:36:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "RandomCalibrationManager.h"

const float RandomCalibrationManager::getNextSample() const
{
    return 0.0f;
}

void RandomCalibrationManager::setSampleRate (float newSampleRate)
{
    // TODO: implement
}

void RandomCalibrationManager::setGainAtFreq (float freq, float gainInDecibels)
{
    // TODO: implement
}

void RandomCalibrationManager::setGainAtIdx (int idx, float gainInDecibels)
{
    // TODO: implement
}

const Curve& RandomCalibrationManager::getCurve() const
{
    return curve;
}

void RandomCalibrationManager::goToNextInterval()
{
    // TODO: implement
}

void RandomCalibrationManager::goToPrevInterval()
{
    // TODO: implement
}
