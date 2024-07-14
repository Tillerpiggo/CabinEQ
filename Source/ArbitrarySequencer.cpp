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
    if (currNoteIdx < 0 || currNoteIdx >= notes.size()) return { 0.0f, 0.0f };
    
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

float ArbitrarySequencer::currentlyPlayingFrequency() const
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size()) return -1;
    return notes.at (currNoteIdx).getFrequency();
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

void ArbitrarySequencer::changeNoteGainAtIdx (int idx, float noteGain)
{
    if (idx < 0 || idx > notes.size())
    {
        std::cerr << "WARNING: changing note gain at idx out of bounds" << std::endl;
        return;
    }
    
    notes.at (idx).setAmplitude(noteGain);
    if (currNoteIdx == idx)
    {
        sineWaveGenerator.setVolume (noteGain);
    }
}

void ArbitrarySequencer::changeNotePanAtIdx (int idx, float notePan)
{
    if (idx < 0 || idx > notes.size())
    {
        std::cerr << "WARNING: changing note gain at idx out of bounds" << std::endl;
        return;
    }
    
    notes.at (idx).setPan(notePan);
    if (currNoteIdx == idx)
    {
        sineWaveGenerator.setPan (notePan);
    }
}

void ArbitrarySequencer::changeNoteGainWithFrequency (float frequency, float noteGain)
{
    for (int i = 0; i < notes.size(); ++i)
    {
        if (notes.at (i).getFrequency() == frequency)
        {
            changeNoteGainAtIdx (i, noteGain);
        }
    }
}

void ArbitrarySequencer::changeNotePanWithFrequency (float frequency, float notePan)
{
    for (int i = 0; i < notes.size(); ++i)
    {
        if (notes.at (i).getFrequency() == frequency)
        {
            changeNotePanAtIdx (i, notePan);
        }
    }
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
