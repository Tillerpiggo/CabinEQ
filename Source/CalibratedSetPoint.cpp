/*
  ==============================================================================

    CalibratedSetPoint.cpp
    Created: 1 Jul 2024 3:33:12pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPoint.h"

void CalibratedSetPoint::calibrateWith (const float value, const CalibrationChoice choice)
{
    if (choice == CalibrationChoice::LowerPreferred)
    {
        upperBound = value;
        
        // If this violates the lower bound, move to a new window
        if (lowerBound.has_value() && lowerBound > value)
        {
            lowerBound = value - windowSize;
        }
    }
    else if (choice == CalibrationChoice::HigherPreferred)
    {
        lowerBound = value;
        
        // Same as above
        if (upperBound.has_value() && upperBound < value)
        {
            upperBound = value + windowSize;
        }
    }
}

float CalibratedSetPoint::estimatedValue()
{
    if (hasEstablishedWindow())
    {
        return (upperBound.value() + lowerBound.value()) / 2.0f;
    }
    else if (lowerBound.has_value())
    {
        return (lowerBound.value() + windowSize) / 2.0f;
    }
    else if (upperBound.has_value())
    {
        return (upperBound.value() - windowSize) / 2.0f;
    }
    else
    {
        return 0.0f;
    }
    
}

bool CalibratedSetPoint::hasEstablishedWindow()
{
    return lowerBound.has_value() && upperBound.has_value();
}

float CalibratedSetPoint::precision()
{
    if (hasEstablishedWindow())
    {
        return (upperBound.value() - lowerBound.value()) / 2.0f;
    }
    else
    {
        return -1; // no window has been established
    }
}
