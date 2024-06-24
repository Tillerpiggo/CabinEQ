/*
  ==============================================================================

    RandomCalibrationManager.cpp
    Created: 23 Jun 2024 6:36:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "RandomCalibrationManager.h"
#include <random>

RandomCalibrationManager::RandomCalibrationManager() : curve (setPointManager)
{
    // Initialize intervalOrder
    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
    {
        intervalOrder.push_back (i);
    }
    
    std::random_device rd;
    std::mt19937 g (rd());
    std::shuffle (intervalOrder.begin(), intervalOrder.end(), g);
    
    // Create and play the first interval
    
}

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

int RandomCalibrationManager::goToNextInterval()
{
    // TODO: implement
}

int RandomCalibrationManager::goToPrevInterval()
{
    // TODO: implement
}
