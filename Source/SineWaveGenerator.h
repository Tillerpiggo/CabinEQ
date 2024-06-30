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
    const std::pair<float, float> getNextSample();
    
    void setNote (Note note);
    void setVolume (float gainInDecibels); // changes the volume of the currently playing note
    void setPan (float panInDecibels); // changes the pan of the currently playing note
    void setPhase (float phaseInRadians); // changes left/right phase relationship of current playing note
    
private:
    static constexpr float TILT = 0.59566214; // 4.5 db/oct slope
    static constexpr float REFERENCE_FREQ = 1000; // freq in hz whexsre amplitudeCompensation = 0
    static const int GAIN_RAMP_LEN_IN_SAMPLES = 3000;
    
    void updatePhaseIncrementAndAmplitudeCompensation();
    
    float sampleRate = 44100;
    std::optional<Note> note;
    std::optional<Note> nextNote;
    
    float phase; // where we are in the sine wave
    
    // == Constants for efficiency ==
    float phaseIncrement = 0;
    float leftAmplitudeCompensation = 0;
    float rightAmplitudeCompensation = 0;
    
    // == Variables to prevent clicking ==
    int endNoteGainRamp = -1;
    int startNoteGainRamp = -1;
};
