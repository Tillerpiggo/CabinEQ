/*
  ==============================================================================

    ArbitrarySequencer.cpp
    Created: 5 Jul 2024 4:43:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitrarySequencer.h"

ArbitrarySequencer::ArbitrarySequencer() {}

std::pair<float, float> ArbitrarySequencer::getNextSample()
{
    SequenceableNote currNote = notes.at (currNoteIdx);
    
    auto [leftSample, rightSample] = sineWaveGenerator.getNextSample();
    auto [leftGain, rightGain] = currNote.getGainAtSample (numSamplesNoteHasBeenPlaying);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= currNote.getDuration())
    {
        goToNextNote();
    }
    
    std::cout << "lSample: " << leftSample << ", rSample: " << rightSample << std::endl;
    std::cout << "leftGain: " << leftGain << ", rightGain: " << rightGain << std::endl;
    
    return { leftSample * leftGain, rightSample * rightGain };
}

void ArbitrarySequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void ArbitrarySequencer::setNotes (std::vector<SequenceableNote> notes)
{
    this->notes = notes;
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx = 0;
    sineWaveGenerator.setNote (getCurrNote().note());
}

void ArbitrarySequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
    {
        currNoteIdx = 0;
    }
    
    sineWaveGenerator.setNote (getCurrNote().note());
}

const SequenceableNote& ArbitrarySequencer::getCurrNote() const
{
    return notes.at (currNoteIdx);
}
