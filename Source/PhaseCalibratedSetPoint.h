/*
  ==============================================================================

    PhaseCalibratedSetPoint.h
    Created: 6 Jul 2024 11:06:18am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "CalibrationChoice.h"

class PhaseCalibratedSetPoint
{
public:
    PhaseCalibratedSetPoint();
    
    void calibrateWith (const CalibrationChoice choice);
    const float estimatedValue() const;
    const float precision() const;
    const float getCurrentGuess() const;
    const float getNextGuess() const;
    
private:
    float currentGuess;
    float range;
    bool hasTriedLeft;
    bool hasTriedRight;
};
