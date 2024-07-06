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
    float value;
    
    std::cout << "lowerBound before: " << lowerBound.value_or (-1) << ", upperBound before: " << upperBound.value_or (-1) << std::endl;
    
    std::cout << "estimatedValue before: " << estimatedValue() << std::endl;
    
    value = estimatedValue();
    
    std::cout << "value: " << estimatedValue() << ", windowSize: " << windowSize << std::endl;
    
    if (choice == CalibrationChoice::HigherPreferred)
    {
        upperBound = value;
        
//        // If this violates the lower bound, move to a new window
//        if (lowerBound.has_value() && lowerBound > value)
//        {
//            lowerBound = value - windowSize;
//        }
    }
    else if (choice == CalibrationChoice::LowerPreferred)
    {
        lowerBound = value;
        
//        // Same as above
//        if (upperBound.has_value() && upperBound < value)
//        {
//            upperBound = value + windowSize;
//        }
    }
    
    std::cout << "lowerBound after: " << lowerBound.value_or (-1) << ", upperBound after: " << upperBound.value_or (-1) << std::endl;
    
    std::cout << "estimatedValue after: " << estimatedValue() << std::endl;
}

void CalibratedSetPoint::setLowerBound (const float lowerBound)
{
    this->lowerBound = lowerBound;
}

void CalibratedSetPoint::setUpperBound (const float upperBound)
{
    this->upperBound = upperBound;
}

const float CalibratedSetPoint::estimatedValue() const
{
    if (hasEstablishedWindow())
    {
        return (upperBound.value() + lowerBound.value()) / 2.0f;
    }
    else if (lowerBound.has_value())
    {
        return (lowerBound.value() + (lowerBound.value() + windowSize)) / 2.0f;
    }
    else if (upperBound.has_value())
    {
        return (upperBound.value() + (upperBound.value() - windowSize)) / 2.0f;
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
    
    if (! lowerBound.has_value() && !upperBound.has_value())
    {
        return std::numeric_limits<float>::max();
    }
    else
    {
        return std::numeric_limits<float>::max() / 2.0f;
    }
}

const float CalibratedSetPoint::getWindowSize() const
{
    if (hasEstablishedWindow())
    {
        return upperBound.value() - lowerBound.value();
    }
    
    return -1;
}

const float CalibratedSetPoint::getLowerBound() const
{
    return lowerBound.value_or (-std::numeric_limits<float>::infinity());
}

const float CalibratedSetPoint::getUpperBound() const
{
    return upperBound.value_or (std::numeric_limits<float>::infinity());
}
