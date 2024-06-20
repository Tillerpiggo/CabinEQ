/*
  ==============================================================================

    Sequencer.cpp
    Created: 20 Jun 2024 12:31:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Sequencer.h"

double Sequencer::getNextSample()
{
    // If we finished the note, switch to the next note
    if (numSamplesNoteHasBeenPlaying >= noteDurationInSamples)
    {
        numSamplesNoteHasBeenPlaying = 0;
        sineWaveGenerator.setNote (melody.getNextNote());
        std::cout << "changed note" << std::endl;
    }
    
    numSamplesNoteHasBeenPlaying++;
    return sineWaveGenerator.getNextSample();
}

void Sequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}
