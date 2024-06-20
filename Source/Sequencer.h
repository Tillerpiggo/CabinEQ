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
#include "Melody.h"

// A basic Sequencer for alternating sine wave and reference tone
class Sequencer
{
public:
    Sequencer()
    {
        Note referenceNote (DEFAULT_GAIN, REFERENCE_FREQ, 0, 0);
        Note controlledNote (DEFAULT_GAIN, DEFAULT_FREQ, 0, 0);
        Note note3 (DEFAULT_GAIN, 1200, 0, 0);
        
        melody.addNote (referenceNote);
        melody.addNote (controlledNote);
        melody.addNote (note3);
        melody.addNote (controlledNote);
    }
    
    double getNextSample();
    void setSampleRate (float newSampleRate);
    
private:
    static constexpr double DEFAULT_GAIN = 12.f;
    static constexpr double DEFAULT_FREQ = 720;
    static constexpr double REFERENCE_FREQ = 1000;
    
    const int noteDurationInSamples = 25000;
    int numSamplesNoteHasBeenPlaying = 0;
    Melody melody;
    SineWaveGenerator sineWaveGenerator;
};
