/*
  ==============================================================================

    IntervalSequencer.cpp
    Created: 23 Jun 2024 7:45:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "IntervalSequencer.h"

float IntervalSequencer::getNextSample()
{
    if (numSamplesNoteHasBeenPlaying >= NOTE_DURATION_IN_SAMPLES)
    {
        numSamplesNoteHasBeenPlaying = 0;
        
        if (isPlayingReferenceFreq)
        {
            sineWaveGenerator.setNote (Note (currGain, currFreq, 0));
        }
        else
        {
            sineWaveGenerator.setNote (Note (REFERENCE_GAIN, REFERENCE_FREQ, 0));
        }
    }
    
    numSamplesNoteHasBeenPlaying++;
    return sineWaveGenerator.getNextSample();
}

void IntervalSequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void IntervalSequencer::setFreq (float freq)
{
    // Also reset interval
    this->currFreq = freq;
    sineWaveGenerator.setNote (Note (currGain, currFreq, 0));
    numSamplesNoteHasBeenPlaying = 0;
    isPlayingReferenceFreq = false;
}

void IntervalSequencer::setGain (float gain)
{
    this->currGain = gain;
    sineWaveGenerator.setVolume (gain);
}
