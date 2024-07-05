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

class ArbitrarySequencer
{
public:
    ArbitrarySequencer();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setNotes (std::vector<SequenceableNote> notes);
    
private:
    void goToNextNote();
    
    SineWaveGenerator sineWaveGenerator;
    std::vector<SequenceableNote> notes;
    
    int currNoteIdx;
    int numSamplesNoteHasBeenPlaying;
};
