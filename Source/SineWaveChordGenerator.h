/*
  ==============================================================================

    SineWaveChordGenerator.h
    Created: 6 Sep 2024 1:37:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitrarySequencer.h"
#include "Curve.h"

/// This takes a curve and lets you repeatedly play chords made out of that curve, sampling some number of sine tones each time the chord plays
class SineWaveChordGenerator
{
public:
    SineWaveChordGenerator();
    
    std::pair<float, float> getNextSample();
    
    void setSampleRate (float newSampleRate);
    
    void setPlayingChord (Curve& curve, float freqToExclude, int numTones = -1); // num tones is max # sine tones to use. If -1, uses all available sine tones (one for every note).
    void updatePlayingChord (Curve& curve, float freqToExclude, int numTones = -1); // same as above, but doesn't restart the chords, and doesn't create new arbitrary sequencers
    
private:
    std::vector<std::vector<std::pair<float, float>>> getNotesToPlay (Curve& curve, int numTonesToPlay);
    void prepareSequencers(); // resets and prepares the arbitrary sequencers with the correct sample rate
    
    std::vector<ArbitrarySequencer> sequencers;
    
    float sampleRate = 0;
    
};
