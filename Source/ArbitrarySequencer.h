/*
  ==============================================================================

    ArbitrarySequencer.h
    Created: 5 Jul 2024 4:43:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SequenceableNote.h"
#include "SineWaveGenerator.h"

class SequencerListener
{
public:
    virtual ~SequencerListener() = default;
    virtual void sequenceDidFinish() = 0;
};

class ArbitrarySequencer
{
public:
    ArbitrarySequencer() : currNoteIdx(0), numSamplesNoteHasBeenPlaying(0), listener(nullptr) {}
    
    std::pair<float, float> getNextSample();
    bool isPlayingFirstNote() const;
    
    void setSampleRate (float newSampleRate);
    void setNotes (const std::vector<SequenceableNote>& notes);
    void setListener (SequencerListener* newListener);
    
private:
    void goToNextNote();
    const SequenceableNote& getCurrNote() const;
    void notifyListener();
    
    SineWaveGenerator sineWaveGenerator;
    std::vector<SequenceableNote> notes;
    
    int currNoteIdx;
    int numSamplesNoteHasBeenPlaying;
    SequencerListener* listener;
};
