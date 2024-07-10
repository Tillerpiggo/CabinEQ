/*
  ==============================================================================

    SliderSequencer.cpp
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSequencer.h"

SliderSequencer::SliderSequencer (const CalibratedSetPointManager& calibratedSetPointManager)
    : calibratedSetPointManager (calibratedSetPointManager) {}

std::pair<float, float> SliderSequencer::getNextSample()
{
    return arbitrarySequencer.getNextSample();
}

void SliderSequencer::setSampleRate (float newSampleRate)
{
    arbitrarySequencer.setSampleRate (newSampleRate);
}

void SliderSequencer::playIntervalAtIdx (int idx)
{
    int noteDurationInSamples = 10000;
    
    SequenceableNote note1 (referenceNote, noteDurationInSamples);
    SequenceableNote note2 (calibratedSetPointManager.noteAt (idx), noteDurationInSamples);
    
    arbitrarySequencer.setNotes ({ note1, note2 });
}
