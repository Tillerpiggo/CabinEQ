/*
  ==============================================================================

    PinkNoiseGenerator.h
    Created: 12 Aug 2024 8:56:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PitchedGenerator.h"
#include "PinkNoise.h"

// Generates banded pink noise at a certain frequency, volume, pan, and phase.
class PinkNoiseGenerator   : public PitchedGenerator
{
public:
    PinkNoiseGenerator();
    
    void setSampleRate (float newSampleRate) override;
    const std::pair<float, float> getNextSample() override;
    
    void setNote (Note note) override;
    void setFrequency (float frequencyInHz) override;
    void setVolume (float volumeInDecibels) override;
    void setPan (float panInDecibels) override;
    void setPhase (float phaseInDecibels) override;
    
private:
    PinkNoise pinkNoise;
    
    float sampleRate = 44100;
    std::optional<Note> note;
};
