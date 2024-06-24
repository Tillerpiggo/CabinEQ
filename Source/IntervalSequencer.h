/*
  ==============================================================================

    IntervalSequencer.h
    Created: 23 Jun 2024 7:45:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"

class IntervalSequencer
{
public:
    IntervalSequencer() {}
    
    float getNextSample();
    void setSampleRate (float newSampleRate);
    void setFreq (float freq);
    void setGain (float gain);
    
private:
    static constexpr float REFERENCE_FREQ = 1000;
    static constexpr float REFERENCE_GAIN = 0.0;
    static const int NOTE_DURATION_IN_SAMPLES = 25000;
    
    float currFreq = 440;
    float currGain = 12.0;
    int numSamplesNoteHasBeenPlaying = 0;
    SineWaveGenerator sineWaveGenerator;
    
    bool isPlayingReferenceFreq = false;
};
