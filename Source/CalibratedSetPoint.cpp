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

void CalibratedSetPoint::calibrateWith (const CalibrationChoice choice, float value)
{
    if (! hasEstablishedWindow())
    {
        lastGuess = value;
    }
    
    switch (choice)
    {
        case CalibrationChoice::HigherPreferred:
            if (hasEstablishedWindow())
            {
                upperBound = value;
            }
            else
            {
                if (! upperBound.has_value())
                {
                    upperBound = value;
                }
                else
                {
                    lowerBound = value;
                }
            }
            break;
        case CalibrationChoice::LowerPreferred:
            lowerBound = value;
            break;
        case CalibrationChoice::NoPreference:
            // If you haven't established a window, don't act on the info yet
            if (! hasEstablishedWindow()) return;
            
            if (upperBound.has_value())
            {
                upperBound = (upperBound.value() + value) / 2.0f;
            }
            else
            {
                upperBound = value + 3.0f;
            }
            
            if (lowerBound.has_value())
            {
                lowerBound = (lowerBound.value() + value) / 2.0f;
            }
            else
            {
                lowerBound = value - 3.0f;
            }
            break;
    }
}

void CalibratedSetPoint::setLowerBound (const float lowerBound)
{
    this->lowerBound = lowerBound;
}

void CalibratedSetPoint::setUpperBound (const float upperBound)
{
    this->upperBound = upperBound;
}

void CalibratedSetPoint::setValue (const float newValue)
{
    upperBound = newValue;
    lowerBound = newValue;
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
    
    if (! lowerBound.has_value() && ! upperBound.has_value())
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

const float CalibratedSetPoint::getNextGuess() const
{
    if (! lastGuess.has_value())
    {
        return 0.0f; // TODO: add interpolation here
    }
    
    if (! upperBound.has_value())
    {
        std::cout << "getting next guess w/ value: " << lastGuess.value() + INIT_WINDOW_SIZE << std::endl;
        return lastGuess.value() + INIT_WINDOW_SIZE;
    }
    else if (! lowerBound.has_value())
    {
        return lastGuess.value() - INIT_WINDOW_SIZE;
    }
    
    return std::numeric_limits<float>::min();
}
