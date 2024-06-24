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
    currIntervalIdx = 0;
    updateSequencer();
}

const float RandomCalibrationManager::getNextSample() const
{
    return 0.0f;
}

void RandomCalibrationManager::setSampleRate (float newSampleRate)
{
    sequencer.setSampleRate (newSampleRate);
}

void RandomCalibrationManager::setCurrGain (float gainInDecibels)
{
    setPointManager.updateGainAtIdx (currIntervalIdx, gainInDecibels);
    sequencer.setGain (gainInDecibels);
}

void RandomCalibrationManager::setGainAtIdx (int i, float gainInDecibels)
{
    setPointManager.updateGainAtIdx (i, gainInDecibels);
}

const Curve& RandomCalibrationManager::getCurve() const
{
    return curve;
}

int RandomCalibrationManager::goToNextInterval()
{
    if (currIntervalIdx >= SetPointManager::NUM_SET_POINTS - 1)
    {
        return -1; // can't go to next so return -1
    }
    
    currIntervalIdx++;
    updateSequencer();
    
    return intervalOrder.at (currIntervalIdx);
}

int RandomCalibrationManager::goToPrevInterval()
{
    if (currIntervalIdx <= 0)
    {
        return -1; // can't go to prev so return -1
    }
    
    currIntervalIdx--;
    updateSequencer();
    
    return intervalOrder.at (currIntervalIdx);
}

void RandomCalibrationManager::updateSequencer()
{
    int currSetPointIdx = intervalOrder.at (currIntervalIdx);
    
    float currFreq = setPointManager.getSetPointFreqs().at (currSetPointIdx);
    float currGain = setPointManager.getSetPointGains().at (currSetPointIdx);
    sequencer.setFreq (currFreq);
    sequencer.setGain (currGain);
}
