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
    
    void calibrateWith (const CalibrationChoice choice, float value);
    void setLowerBound (const float lowerBound);
    void setUpperBound (const float upperBound);
    
    void setValue (const float newValue);
    
    const bool hasEstablishedWindow() const; // if it has set a definitive upper and lower bound yet
    const float estimatedValue() const;
    const float precision() const;
    const float getWindowSize() const;
    const float getLowerBound() const;
    const float getUpperBound() const;
    const int getInitialTempo() const;
    
private:
    static constexpr float INIT_WINDOW_SIZE = 12.0f;
    
    std::optional<float> lowerBound;
    std::optional<float> upperBound;
    float nextGuess = 0.0f;
    
    float windowSize;
};
