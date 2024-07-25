/*
  ==============================================================================

    SliderSetPointManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SetPointManager.h"

SetPointManager::SetPointManager()
{
    // Generate NUM_PTS evenly spaced frequencies from 20 to 15000khz
    setPoints.clear();
    setPoints.reserve (NUM_PTS);
    
    float startFreq = 50.0f;
    float endFreq = 950.0f;

    float logStart = std::log10(startFreq);
    float logEnd = std::log10(endFreq);
    float step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        float logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        
        setPoints.emplace_back (freq, 0, 0);
    }
    
    startFreq = 1050.0f;
    endFreq = 7500.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        setPoints.emplace_back (freq, 0, 0);
    }
    
    startFreq = 8000.0f;
    endFreq = 18000.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        setPoints.emplace_back (freq, 0, 0);
    }
    
    curve.updateWithSetPoints (setPoints);
}

void SetPointManager::setSetPointAt (int idx, SetPoint setPoint)
{
    if (idx < 0 || idx >= getNumPoints()) 
        return;
    setPoints[idx] = setPoint;
    curve.updateWithSetPoints (setPoints);
}

void SetPointManager::setAmplitudeAt (int idx, float newAmplitude)
{
    if (idx < 0 || idx >= getNumPoints()) 
        return;
    setPoints[idx].amplitude = newAmplitude;
    curve.updateWithSetPoints (setPoints);
}

void SetPointManager::setPanAt (int idx, float newPan)
{
    if (idx < 0 || idx >= getNumPoints()) 
        return;
    setPoints[idx].pan = newPan;
    curve.updateWithSetPoints (setPoints);
}

const Curve& SetPointManager::getCurve() const
{
    return curve;
}
