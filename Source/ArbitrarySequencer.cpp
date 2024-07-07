/*
  ==============================================================================

    ArbitrarySequencer.cpp
    Created: 5 Jul 2024 4:43:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitrarySequencer.h"

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
    
    return { leftSample * leftGain, rightSample * rightGain };
}

bool ArbitrarySequencer::isPlayingFirstNote() const
{
    return currNoteIdx == 0;
}

void ArbitrarySequencer::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void ArbitrarySequencer::setNotes (const std::vector<SequenceableNote>& newNotes)
{
    notes = newNotes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    
    sineWaveGenerator.setNote (getCurrNote().note());
}

void ArbitrarySequencer::setListener(SequencerListener* newListener)
{
    listener = newListener;
}

void ArbitrarySequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
    {
        currNoteIdx = 0;
        notifyListener();
    }
    
    sineWaveGenerator.setNote (getCurrNote().note());
}

const SequenceableNote& ArbitrarySequencer::getCurrNote() const
{
    return notes.at (currNoteIdx);
}

void ArbitrarySequencer::notifyListener()
{
    if (listener != nullptr)
    {
        listener->sequenceDidFinish();
    }
}
