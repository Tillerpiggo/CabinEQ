/*
  ==============================================================================

    CalibratedSetPoint.cpp
    Created: 22 Jun 2024 1:20:01pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPoint.h"

const double CalibratedSetPoint::estimatedValue() const
{
    return (upperBound + lowerBound) / 2.0;
}

void CalibratedSetPoint::calibrateWith (CalibrationManager::Choice choice)
{
    if (choice == CalibrationManager::Choice::LowerPreferred)
    {
        upperBound = estimatedValue();
    }
    else
    {
        lowerBound = estimatedValue();
    }
}
