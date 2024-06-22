/*
  ==============================================================================

    CalibrationSequence.cpp
    Created: 22 Jun 2024 11:08:18am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationSequence.h"

CalibrationSequence::CalibrationSequence()
{
    createHappyBirthdayMelody();
}

// Dummy implementation
const Melody& CalibrationSequence::getNextMelody() const
{
    return happyBirthdayMelody;
}

void CalibrationSequence::createHappyBirthdayMelody() {
    // Notes for "Happy Birthday" (simple version)
    // Assuming note values (e.g., MIDI note numbers) for simplicity
    // Happy Birthday to You
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (62); // D4
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (65); // F4
    happyBirthdayMelody.addNote (64); // E4

    // Happy Birthday to You
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (62); // D4
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (67); // G4
    happyBirthdayMelody.addNote (65); // F4

    // Happy Birthday dear [Name]
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (60); // C4
    happyBirthdayMelody.addNote (72); // C5
    happyBirthdayMelody.addNote (69); // A4
    happyBirthdayMelody.addNote (65); // F4
    happyBirthdayMelody.addNote (64); // E4
    happyBirthdayMelody.addNote (62); // D4

    // Happy Birthday to You
    happyBirthdayMelody.addNote (70); // A4
    happyBirthdayMelody.addNote (70); // A4
    happyBirthdayMelody.addNote (69); // A4
    happyBirthdayMelody.addNote (65); // F4
    happyBirthdayMelody.addNote (67); // G4
    happyBirthdayMelody.addNote (65); // F4
    
    happyBirthdayMelody.setControlledNote (70); // C4
}
