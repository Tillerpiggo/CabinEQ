/*
  ==============================================================================

    SliderSequencer.cpp
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSequencer.h"

void SliderSequencer::playInterval (float frequency, float amplitude)
{
    int noteDurationInSamples = 10000;
    
    SequenceableNote note1 (referenceNote, noteDurationInSamples);
    SequenceableNote note2 (Note (frequency, amplitude, 0.0f, 0.0f), noteDurationInSamples);
    
    arbitrarySequencer.setNotes ({ note1, note2 });
}

void SliderSequencer::changeControlledAmplitude (float newAmplitude)
{
    // change arbitrarysequencer to be able to dynamically change volume of note at index
    arbitrarySequencer.changeNoteGainAtIdx (1, newAmplitude);
}
