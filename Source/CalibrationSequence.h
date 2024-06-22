/*
  ==============================================================================

    CalibrationSequence.h
    Created: 22 Jun 2024 11:08:18am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Melody.h"

class CalibrationSequence
{
public:
    CalibrationSequence();
    
    const Melody& getNextMelody() const;
    
    // TODO: Hardcode a list of melodies to return here
private:
    Melody happyBirthdayMelody;
    void createHappyBirthdayMelody();
};
