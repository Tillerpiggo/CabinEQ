/*
  ==============================================================================

    MelodicNoiseSequencer.h
    Created: 27 Oct 2024 1:05:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"
#include "SpatialPatternGenerator.h" // for NoiseNote
#include "SpatialPinkNoiseGenerator.h"
#include "SineWaveGenerator.h"

class MelodicNoiseSequencer
{
public:
    MelodicNoiseSequencer();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (std::vector<NoiseNote> notes);
    void setSineVolume (float sineVolume);
    void setSpeedFactor (float speedFactor);
    void setOctaveRange (float octaveRange);
    std::pair<float, float> getNextSample();
    
private:
    NoiseNote getCurrNote();
    void updateFilters();
    void goToNextNote();
    
    std::vector<NoiseNote> notes;
    
    PinkNoise lowerNoise;
    PinkNoise upperNoise;
    PinkNoise noteNoise;
    
    int numSamplesNoteHasBeenPlaying;
    int currNoteIdx;
    float sampleRate;
    
    juce::dsp::IIR::Filter<float> notchFilter; // to add a notch in the main noise
    juce::dsp::IIR::Filter<float> bandpassFilter;
    juce::dsp::IIR::Filter<float> lowPassFilter;
    juce::dsp::IIR::Filter<float> highPassFilter;
    SineWaveGenerator sineWaveGenerator;
    SpatialPinkNoiseGenerator spatialPinkNoiseGenerator;
    int snapToZeroCounter = 0;
    float sineVolume = 0.0f; // in db
    float speedFactor = 1.0f;
    float freqOffsetFactor = 1.0f;
    float octaveRange = 3.0f; // +- octaves of randomization
};
