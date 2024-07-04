/*
  ==============================================================================

    CalibratedSetPoint.h
    Created: 1 Jul 2024 3:33:12pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CalibrationChoice.h"

class CalibratedSetPoint
{
public:
    CalibratedSetPoint (float windowSize);
    CalibratedSetPoint (float lowerBound, float upperBound);
    
    void calibrateWith (const CalibrationChoice choice);
    void setWindow (float lowerBound, float upperBound);
    const bool hasEstablishedWindow() const; // if it has set a definitive upper and lower bound yet
    const float estimatedValue() const;
    const float precision() const;
    const float getWindowSize() const;
    
private:
    std::optional<float> lowerBound;
    std::optional<float> upperBound;
    
    float windowSize;
};
