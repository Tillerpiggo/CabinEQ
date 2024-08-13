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
    ArbitrarySequencer();
    
    std::pair<float, float> getNextSample();
    bool isPlayingFirstNote() const;
    float currentlyPlayingFrequency() const;
    
    void setSampleRate (float newSampleRate);
    void setNotes (const std::vector<SequenceableNote>& notes, bool repeating = true);
    void setListener (SequencerListener* newListener);
    
    void changeNoteAtIdx (int idx, Note newNote);
    void changeNoteGainAtIdx (int idx, float noteGain);
    void changeNotePanAtIdx (int idx, float notePan);
    void changeNoteGainWithFrequency (float frequency, float noteGain);
    void changeNotePanWithFrequency (float frequency, float notePan);
    
private:
    void goToNextNote();
    const SequenceableNote& getCurrNote() const;
    void notifyListener();
    
    SineWaveGenerator sineWaveGenerator;
    std::vector<SequenceableNote> notes;
    
    int currNoteIdx;
    int numSamplesNoteHasBeenPlaying;
    SequencerListener* listener;
    
    bool isRepeating = true;
};
