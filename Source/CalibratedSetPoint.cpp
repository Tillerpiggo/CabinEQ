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
    isCompletelyUncalibrated = false;
    
    switch (choice)
    {
        case CalibrationChoice::HigherPreferred:
        {
            upperBound = value;
            
            // Make bias 1 at
            float L = 1;
            float k = 0.4;  // Adjust the steepness
            float x_0 = 2;  // Adjust the midpoint
            bias = L / (1 + std::exp (-k * (numNoPreferencesInARow - x_0)));
            
            numNoPreferencesInARow = 0;
            break;
        }
        case CalibrationChoice::LowerPreferred:
        {
            lowerBound = value;
            
            // Make bias 1 at
            float L = 1;
            float k = 0.4;  // Adjust the steepness
            float x_0 = 2;  // Adjust the midpoint
            bias = 1.0f - (L / (1 + std::exp (-k * (numNoPreferencesInARow - x_0))));
            
            numNoPreferencesInARow = 0;
            break;
        }
        case CalibrationChoice::NoPreference:
            numNoPreferencesInARow++;
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
        //std::cout << "estimatedValue: " << (upperBound.value() + lowerBound.value()) / 2.0f << std::endl;
        return ((bias) * upperBound.value() + (1.0f - bias) * lowerBound.value());
    }
    else if (lowerBound.has_value())
    {
        //std::cout << "estimatedValue: " << (lowerBound.value() + (lowerBound.value() + windowSize)) / 2.0f << std::endl;
        return ((1.0f - bias) * lowerBound.value() + (bias) * (lowerBound.value() + windowSize));
    }
    else if (upperBound.has_value())
    {
        //std::cout << "estimatedValue: " << (upperBound.value() + (upperBound.value() - windowSize)) / 2.0f << std::endl;
        return ((bias) * upperBound.value() + (1.0f - bias) * (upperBound.value() - windowSize));
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

const bool CalibratedSetPoint::getIsCompletelyUncalibrated() const
{
    return isCompletelyUncalibrated;
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

const int CalibratedSetPoint::getInitialTempo() const
{
    if (! hasEstablishedWindow())
    {
        return 0;
    }
    else
    {
        int tempo = std::log2(12.0 / precision());
        if (tempo < 0) tempo = 0;

        return tempo;
    }
}
