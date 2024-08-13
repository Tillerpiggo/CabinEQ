/*
  ==============================================================================

    SineWaveGenerator.h
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PitchedGenerator.h"

// Generates sine waves at a certain frequency, volume, pan, and phase.
class SineWaveGenerator   : public PitchedGenerator
{
public:
    SineWaveGenerator();
    
    void setSampleRate (float newSampleRate) override;
    const std::pair<float, float> getNextSample() override;
    
    void setNote (Note note) override;
    void setFrequency (float frequencyInHz) override;
    void setVolume (float volumeInDecibels) override;
    void setPan (float panInDecibels) override;
    void setPhase (float phaseInRadians) override;
    
private:
    static constexpr float REFERENCE_FREQ = 1000; // freq in hz whexsre amplitudeCompensation = 0
    
    void updatePhaseIncrementAndAmplitudeCompensation();
    
    float sampleRate = 44100;
    std::optional<Note> note;
    
    float phase = 0; // where we are in the sine wave
    
    // == Constants for efficiency ==
    float phaseIncrement = 0;
    float leftAmplitudeCompensation = 0;
    float rightAmplitudeCompensation = 0;
    
    // == Vars to reduce clicking
    static constexpr float FREQ_STEP = 1.0001f;
    static constexpr float AMPL_STEP = 1.0001f;
    std::optional<float> targetFrequency;
    std::optional<float> targetAmplitude;
};
