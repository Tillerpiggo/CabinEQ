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

void CalibrationManager::chooseOption (CalibrationChoice choice)
{
    // Calibrate the relevant (current) choice
    setPoints[currentSetPointFreq].calibrateWith (choice);
    
    // Move to the next melody
    Melody melody = calibrationSequence.getNextMelody();
    changeMelodyTo (melody);
}

float CalibrationManager::getNextSample()
{
    return sequencer.getNextSample();
}

void CalibrationManager::changeMelodyTo (const Melody& melody)
{
    // Add new set point if we don't have one for the controlled frequency
    if (setPoints.find (melody.getControlledFrequency()) == setPoints.end())
    {
        float variance = 12.0; // dB range to binary search under for new set point
        float estimatedGain = curve.valueAtFrequency (melody.getControlledFrequency()).real();
        //CalibratedSetPoint newSetPoint (estimatedGain - variance, estimatedGain + variance);
        //setPoints[melody.getControlledFrequency()] = newSetPoint;
    }
    
    // Create a NoteSequence from the melody
    NoteSequence noteSequence;
    
    for (int noteVal : melody.getNotes())
    {
        float freq = melody.freqForNote (noteVal);
        float gain = curve.valueAtFrequency (melody.freqForNote (noteVal)).real();
        
        Note note (gain, freq, 0.0);
        noteSequence.addNote (note);
    }
    
    // Move to that melody
    sequencer.queueNextNoteSequence (noteSequence);
}


