/*
  ==============================================================================

    CalibrationManager.cpp
    Created: 22 Jun 2024 11:08:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationManager.h"

void CalibrationManager::setDelegate (CalibrationManagerDelegate* delegate) 
{
    this->delegate = delegate;
}

void CalibrationManager::chooseOption (Choice choice)
{
    // Calibrate the relevant (current) choice
    setPoints[currentSetPointFreq].calibrateWith (choice);
    
    // Move to the next melody
    Melody melody = calibrationSequence.getNextMelody();
    changeMelodyTo (melody);
}

double CalibrationManager::getNextSample()
{
    return sequencer.getNextSample();
}

void CalibrationManager::changeMelodyTo (const Melody& melody)
{
    // Add new set point if we don't have one for the controlled frequency
    if (setPoints.find (melody.getControlledFrequency()) == setPoints.end())
    {
        double variance = 12.0; // dB range to binary search under for new set point
        double estimatedGain = curve.valueAtFrequency (melody.getControlledFrequency()).real();
        CalibratedSetPoint newSetPoint (estimatedGain - variance, estimatedGain + variance);
        setPoints[melody.getControlledFrequency()] = newSetPoint;
    }
    
    // Create a NoteSequence from the melody
    NoteSequence noteSequence;
    
    for (int noteVal : melody.getNotes())
    {
        double freq = melody.freqForNote (noteVal);
        double gain = curve.valueAtFrequency (melody.freqForNote (noteVal)).real();
        
        Note note (gain, freq, 0.0);
        noteSequence.addNote (note);
    }
    
    // Move to that melody
    sequencer.queueNextNoteSequence (noteSequence);
}


