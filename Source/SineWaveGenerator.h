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
    SineWaveGenerator ();
    
    void setSampleRate (float newSampleRate);
    float getNextSample();
    
    void setNote (Note note);
    void setVolume (float gainInDecibels); // changes the volume of the currently playing note
    
private:
    static constexpr float TILT = 0.62; // 0.5 ~ pink noise, 0.6 ~ equal loudness, 0.4 ~ bassier
    static constexpr float REFERENCE_FREQ = 1000; // freq in hz where amplitudeCompensation = 0
    
    void updatePhaseIncrementAndAmplitudeCompensation();
    
    float sampleRate = 0; // so it crashes if we try to run before setting sample rate
    std::optional<Note> note;
    
    float phase; // where we are in the sine wave
    
    // == Constants for efficiency ==
    float phaseIncrement = 0;
    float amplitudeCompensation = 0;
    
    // == Variables to prevent clicking ==
    //int rampLengthInSamples;
};
