/*
  ==============================================================================

    Sequencer.cpp
    Created: 20 Jun 2024 12:31:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Sequencer.h"

float Sequencer::getNextSample()
{
    // If we finished the note, switch to the next note
    if (numSamplesNoteHasBeenPlaying >= noteDurationInSamples)
    {
        numSamplesNoteHasBeenPlaying = 0;
        
        std::cout << "nextNoteSequence: " << nextNoteSequence.has_value() << ", noteIdx: " << noteSequence.getNoteIdx();
        
        // Change to next note sequence if we're at the start of a new cycle
        if (noteSequence.getNoteIdx() == 0 && nextNoteSequence)
        {
            noteSequence = nextNoteSequence.value();
            nextNoteSequence.reset();
        }
        
        Note nextNote = noteSequence.getNextNote();
        std::cout << ", nextNote volume: " << nextNote.gain << std::endl;
        sineWaveGenerator.setNote (nextNote);
    }
    
    numSamplesNoteHasBeenPlaying++;
    return sineWaveGenerator.getNextSample();
}

void Sequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void Sequencer::queueNextNoteSequence (NoteSequence nextNoteSequence)
{
    this->nextNoteSequence = nextNoteSequence;
}
