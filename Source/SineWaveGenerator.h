/*
  ==============================================================================

    SineWaveGenerator.h
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Note.h"

class SineWaveGenerator
{
public:
    SineWaveGenerator (Note note) : note (note) {};
    
    void setSampleRate (double newSampleRate);
    double getNextSample();
    
    void setNote (Note note);
    
private:
    static constexpr double TILT = 0.6; // 0.5 ~ pink noise, 0.6 ~ equal loudness, 0.4 ~ bassier
    static constexpr double REFERENCE_FREQ = 1000; // freq in hz where amplitudeCompensation = 0
    
    void updatePhaseIncrementAndAmplitudeCompensation();
    
    double sampleRate = 0; // so it crashes if we try to run before setting sample rate
    Note note;
    
    double phase; // where we are in the sine wave
    
    // == Constants for efficiency ==
    double phaseIncrement = 0;
    double amplitudeCompensation = 0;
    
    // == Variables to prevent clicking ==
    int gainRamp = 0;
};
