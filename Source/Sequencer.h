/*
  ==============================================================================

    Sequencer.h
    Created: 20 Jun 2024 12:31:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"
#include "NoteSequence.h"

// A basic Sequencer for alternating sine wave and reference tone
class Sequencer
{
public:
    Sequencer()
    {
        Note referenceNote (DEFAULT_GAIN, REFERENCE_FREQ, 0);
        Note controlledNote (DEFAULT_GAIN, DEFAULT_FREQ, 0);
        Note note3 (DEFAULT_GAIN, 1200, 0);
        
        noteSequence.addNote (referenceNote);
        noteSequence.addNote (controlledNote);
        noteSequence.addNote (note3);
        noteSequence.addNote (controlledNote);
    }
    
    float getNextSample();
    void setSampleRate (float newSampleRate);
    void queueNextNoteSequence (NoteSequence nextNoteSequence);
    
private:
    static constexpr float DEFAULT_GAIN = 12.f;
    static constexpr float DEFAULT_FREQ = 720;
    static constexpr float REFERENCE_FREQ = 1000;
    
    const int noteDurationInSamples = 25000;
    int numSamplesNoteHasBeenPlaying = 0;
    NoteSequence noteSequence;
    std::optional<NoteSequence> nextNoteSequence;
    SineWaveGenerator sineWaveGenerator;
};
