/*
  ==============================================================================

    RandomCalibrationManager.cpp
    Created: 23 Jun 2024 6:36:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "RandomCalibrationManager.h"
#include <random>

RandomCalibrationManager::RandomCalibrationManager() : gainCurve (gainSetPointManager), balanceCurve (balanceSetPointManager)
{
    // Initialize intervalOrder
    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
    {
        intervalOrder.push_back (i);
    }
    
//    std::random_device rd;
//    std::mt19937 g (rd());
//    std::shuffle (intervalOrder.begin(), intervalOrder.end(), g);
    
    // Create and play the first interval
    currIntervalIdx = 0;
    updateSequencer();
}

const std::pair<float, float> RandomCalibrationManager::getNextSample()
{
    return intervalSequencer.getNextSample();
}

void RandomCalibrationManager::setSampleRate (float newSampleRate)
{
    intervalSequencer.setSampleRate (newSampleRate);
}

void RandomCalibrationManager::setCurrGain (float gainInDecibels)
{
    gainSetPointManager.updateValueAtIdx (intervalOrder.at (currIntervalIdx), gainInDecibels);
    intervalSequencer.setGain (gainInDecibels);
}

void RandomCalibrationManager::setCurrPan (float balanceGainInDecibels)
{
    balanceSetPointManager.updateValueAtIdx (intervalOrder.at (currIntervalIdx), balanceGainInDecibels);
    intervalSequencer.setPan (balanceGainInDecibels);
}

void RandomCalibrationManager::setGainAtIdx (int i, float gainInDecibels)
{
    gainSetPointManager.updateValueAtIdx (i, gainInDecibels);
}

void RandomCalibrationManager::setBalanceAtIdx (int i, float balanceGainInDecibels)
{
    gainSetPointManager.updateValueAtIdx (i, balanceGainInDecibels);
}

const Curve& RandomCalibrationManager::getGainCurve() const
{
    return gainCurve;
}

const Curve& RandomCalibrationManager::getBalanceCurve() const
{
    return balanceCurve;
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
    
    float currFreq = gainSetPointManager.getSetPointFreqs().at (currSetPointIdx);
    float currGain = gainSetPointManager.getSetPointGains().at (currSetPointIdx);
    intervalSequencer.setFreq (currFreq);
    intervalSequencer.setGain (currGain);
}
