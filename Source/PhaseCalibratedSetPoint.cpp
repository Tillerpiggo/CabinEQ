/*
  ==============================================================================

    PhaseCalibratedSetPoint.cpp
    Created: 6 Jul 2024 11:06:18am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PhaseCalibratedSetPoint.h"

PhaseCalibratedSetPoint::PhaseCalibratedSetPoint()
: currentGuess (0.0f), range (M_PI / 32.0f), hasTriedLeft (false), hasTriedRight (false) {}

void PhaseCalibratedSetPoint::calibrateWith (const CalibrationChoice choice)
{
    std::cout << "Calibrating phase" << std::endl;
    std::cout << "current guess before: " << currentGuess;
    std::cout << ", range: " << range << std::endl;
    // Note: Make LowerPreferred always correspond to the current guess
    if (choice == CalibrationChoice::LowerPreferred)
    {
        std::cout << "Calibrating phase with LowerPreferred" << std::endl;
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
        std::cout << "Calibrating phase with HigherPreferred" << std::endl;
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
    
    std::cout << "currentGuess: " << currentGuess;
    std::cout << ", range: " << range << std::endl;
    
}

const float PhaseCalibratedSetPoint::estimatedValue() const
{
    return currentGuess;
}

const float PhaseCalibratedSetPoint::precision() const
{
    return range;
}

const float PhaseCalibratedSetPoint::getCurrentGuess() const
{
    return currentGuess;
}

const float PhaseCalibratedSetPoint::getNextGuess() const
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

