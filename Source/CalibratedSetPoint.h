/*
  ==============================================================================

    CalibratedSetPoint.h
    Created: 22 Jun 2024 1:20:01pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibrationChoice.h"

class CalibratedSetPoint
{
public:
    CalibratedSetPoint() : lowerBound (-24.0f), upperBound (-48.0f) {}
    CalibratedSetPoint (float lowerBound, float upperBound)
    : lowerBound (lowerBound), upperBound (upperBound) {}
    
    const float estimatedValue() const;
    void calibrateWith (CalibrationChoice choice);
    
private:
    float lowerBound; // in dB; 0 = no compensation
    float upperBound; // in dB; 0 = no compensation
};
