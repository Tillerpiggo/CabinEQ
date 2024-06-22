/*
  ==============================================================================

    CalibratedSetPoint.h
    Created: 22 Jun 2024 1:20:01pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibrationManager.h"

class CalibratedSetPoint
{
public:
    CalibratedSetPoint (double lowerBound, double upperBound)
    : upperBound (upperBound), lowerBound (lowerBound) {}
    
    const double estimatedValue() const;
    void calibrateWith (CalibrationManager::Choice choice);
    
private:
    double upperBound; // in dB; 0 = no compensation
    double lowerBound; // in dB; 0 = no compensation
};
