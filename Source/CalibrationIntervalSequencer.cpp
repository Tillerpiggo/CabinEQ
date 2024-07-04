/*
  ==============================================================================

    CalibrationIntervalSequencer.cpp
    Created: 1 Jul 2024 3:35:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationIntervalSequencer.h"

const std::pair<float, float> CalibrationIntervalSequencer::getNextSample()
{
    // Make sure notes have been set, otherwise return silence
    if (! note1.has_value() || ! note2.has_value())
    {
        return { 0.0f, 0.0f };
    }
    
    if (numSamplesNoteHasBeenPlaying >= NOTE_DURATION_IN_SAMPLES)
    {
        numSamplesNoteHasBeenPlaying = 0;
        
        if (isPlayingFirstNote())
        {
            sineWaveGenerator.setNote (note2.value());
            isFirstNotePlaying = false;
        }
        else
        {
            sineWaveGenerator.setNote (note1.value());
            isFirstNotePlaying = true;
        }
    }
    
    numSamplesNoteHasBeenPlaying++;
    return sineWaveGenerator.getNextSample();
}

void CalibrationIntervalSequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void CalibrationIntervalSequencer::setNotes (Note note1, Note note2)
{
    this->note1 = note1;
    this->note2 = note2;
    
    // Instantly switch to the new note sequence
    numSamplesNoteHasBeenPlaying = 0;
    sineWaveGenerator.setNote (note1);
    isFirstNotePlaying = true;
}
 
bool CalibrationIntervalSequencer::isPlayingFirstNote() const
{
    return isFirstNotePlaying;
}
