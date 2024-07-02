/*
  ==============================================================================

    CalibratedSetPoint.h
    Created: 1 Jul 2024 3:33:12pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CalibrationManager.h"

class CalibratedSetPoint
{
public:
    CalibratedSetPoint (float windowSize) : windowSize (windowSize) {}
    
    void calibrateWith (const float value, const CalibrationChoice choice);
    const bool hasEstablishedWindow() const; // if it has set a definitive upper and lower bound yet
    const float estimatedValue() const;
    const float precision() const;
    
private:
    std::optional<float> lowerBound;
    std::optional<float> upperBound;
    
    float windowSize;
};
