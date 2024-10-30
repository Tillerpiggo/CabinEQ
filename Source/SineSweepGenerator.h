/*
  ==============================================================================

    SineSweepGenerator.h
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"
#include "Curve.h"
#include "SweepPattern.h"


/// This class provides an easy interface to generate a sine sweep in a given frequency range.
class SineSweepGenerator
{
public:
    SineSweepGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setSweepPattern (SweepPattern sweepPattern); // must be called before getNextSample is called for audio output
    float getCurrFreq() const;
    
private:
    SineWaveGenerator sineWaveGenerator;
    std::optional<SweepPattern> sweepPattern;
};
