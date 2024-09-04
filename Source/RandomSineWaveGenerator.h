/*
  ==============================================================================

    RandomSineWaveGenerator.h
    Created: 27 Aug 2024 11:12:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"
#include "SequenceableNote.h"
#include "Curve.h"

// generates random sine waves at each frequency over and over again, changing live with a curve
class RandomSineWaveGenerator
{
public:
    RandomSineWaveGenerator();
    
    void setSampleRate (float newSampleRate);
    void setCurve (Curve curve);
    const std::pair<float, float> getNextSample();
    
private:
    void randomizeNote();
    
    SineWaveGenerator sineWaveGenerator;
    std::optional<Curve> curve;
    SequenceableNote currNote = SequenceableNote (1000.0f, 0.0f, 0.0f, 0.0f, 1000);
    int noteLengthInSamples = 1000;
    int samplesNoteHasBeenPlaying = 0;
};
