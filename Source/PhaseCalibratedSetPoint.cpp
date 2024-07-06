/*
  ==============================================================================

    PhaseCalibratedSetPoint.cpp
    Created: 6 Jul 2024 11:06:18am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PhaseCalibratedSetPoint.h"

PhaseCalibratedSetPoint::PhaseCalibratedSetPoint()
: currentGuess (0.0f), range (M_PI), hasTriedLeft (true), hasTriedRight (false) {}

void PhaseCalibratedSetPoint::calibrateWith (const CalibrationChoice choice)
{
    // Note: Make LowerPreferred always correspond to the current guess
    if (choice == CalibrationChoice::LowerPreferred)
    {
        // Update hasTriedLeft && hasTriedRight
        if (! hasTriedLeft)
        {
            hasTriedLeft = true;
        }
        else if (! hasTriedRight)
        {
            hasTriedRight = true;
        }
        
        // If we've tried both sides, reduce range and keep trying
        if (hasTriedLeft && hasTriedRight)
        {
            hasTriedLeft = false;
            hasTriedRight = false;
            range /= 2.0f;
        }
    }
    else if (choice == CalibrationChoice::HigherPreferred)
    {
        if (hasTriedLeft)
        {
            currentGuess -= range;
        }
        else if (hasTriedRight)
        {
            currentGuess += range;
        }
        
        hasTriedLeft = false;
        hasTriedRight = false;
        range /= 2.0f;
    }
}

const float PhaseCalibratedSetPoint::estimatedValue() const
{
    return currentGuess;
}

const float PhaseCalibratedSetPoint::nextGuess() const
{
    if (! hasTriedLeft)
    {
        return currentGuess - range;
    }
    else
    {
        return currentGuess + range;
    }
}

const float PhaseCalibratedSetPoint::precision() const
{
    return range;
}

