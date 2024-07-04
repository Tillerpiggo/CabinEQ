/*
  ==============================================================================

    CalibratedSetPoint.cpp
    Created: 1 Jul 2024 3:33:12pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPoint.h"

CalibratedSetPoint::CalibratedSetPoint (float windowSize) : windowSize (windowSize)
{
}

CalibratedSetPoint::CalibratedSetPoint (float lowerBound, float upperBound) : windowSize ((upperBound - lowerBound) / 2.0f)
{
    lowerBound = lowerBound;
    upperBound = upperBound;
}

void CalibratedSetPoint::calibrateWith (const CalibrationChoice choice)
{
    float value = estimatedValue();
    if (choice == CalibrationChoice::HigherPreferred)
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

const float CalibratedSetPoint::estimatedValue() const
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

const bool CalibratedSetPoint::hasEstablishedWindow() const
{
    return lowerBound.has_value() && upperBound.has_value();
}

const float CalibratedSetPoint::precision() const
{
    if (hasEstablishedWindow())
    {
        return getWindowSize() / 2.0f;
    }
    
    return -1;
}

const float CalibratedSetPoint::getWindowSize() const
{
    if (! hasEstablishedWindow())
    {
        return upperBound.value() - lowerBound.value();
    }
    
    return -1;
}
