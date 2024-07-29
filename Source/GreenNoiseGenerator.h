/*
  ==============================================================================

    GreenNoiseGenerator.h
    Created: 29 Jul 2024 2:43:35pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitrarySequencer.h"

/// Generates sine tones playing over white noise. Should probably be filtered through something before playback occurs (or it'll sound really harsh)
class GreenNoiseGenerator
{
public:
    GreenNoiseGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void setCenterFrequency (float centerFreq);
    float getCenterFrequency() const;
    
private:
    void updateArbitrarySequencerNotes();
    
    ArbitrarySequencer arbitrarySequencer;
    juce::Random random;
    
    float centerFreq = 1000.0f;
    
};
