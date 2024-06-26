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
    static constexpr float TILT = 0.59566214; // 0.5 ~ too bright, 0.62 ~ way too bassy, 0.55 ~ still too bright, 0.58 ~ pretty good but maybe slightly too bright (but at the same time sound darker than the speakers)
    // NVM 0.581 seems too bassy
    // ~ 0.59 - a bit too bassy
    // ~ 0.57 still too bassy a little? Just not enough vocal presence
    // ~ 0.56 still bass heavy?? or maybe not at all?
    // ~ I think 0.55 might actually be right
    // ~ 0.53 is too bright still
    static constexpr float REFERENCE_FREQ = 1000; // freq in hz whexsre amplitudeCompensation = 0
    static const int GAIN_RAMP_LEN_IN_SAMPLES = 500;
    
    void updatePhaseIncrementAndAmplitudeCompensation();
    
    float sampleRate = 44100;
    std::optional<Note> note;
    std::optional<Note> nextNote;
    
    float phase; // where we are in the sine wave
    
    // == Constants for efficiency ==
    float phaseIncrement = 0;
    float amplitudeCompensation = 0;
    
    // == Variables to prevent clicking ==
    int endNoteGainRamp = -1;
    int startNoteGainRamp = -1;
};
