/*
  ==============================================================================

    SquareGenerator.h
    Created: 10 Feb 2025 3:15:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"

class SquareGenerator
{
public:
    SquareGenerator();
    
    std::pair<float, float> getNextSample();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setGridDimensions (int numRows, int numCols);
    void setCheckerboardCoords (int freqIdx, int panIdx);
    void setVolumeGain (float volumeGain);
    void setMinFreq (float minFreq);
    void setBandpassRange (float bottom, float top); // bottom and top from [0, 1]. Bottom < Top
    
private:
    void updateGeneratorsIfNeeded(); // recalculates generators to match state if shouldUpdateGenerators is true, and computes but doesn't apply minFreq and maxFreq
    void updateFiltersIfNeeded(); // updates filters according to minFreq/maxFreq/bottom/top
    
    float sampleRate;
    std::vector<std::pair<float, float>> leftRightGains; // stores the left and right gains for all noise gens
    float totalGain = 1.0f;
    
    // Pink noise generation
//    std::vector<PinkNoise> pinkNoises;
    juce::Random random;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFiltersLeft;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFiltersRight;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFiltersLeft;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFiltersRight;
    int order = 6;
    int snapToZeroCounter = 0;
    
    // Add ramp when starting from nothing
    int rampSamples = 5000;
    int currRampSample = 0; // 0 = ramp start, > 1000 = ramp ended
    
    int numRows = 2;
    int numCols = 2;
    int freqIdx = 0; // the frequency idx from [0, resolution). Panning width assumed to match resolution width. 0 is lowest and resolution is highest.
    int panIdx = 0; // the pan idx from [0, resolution]. 0 is farthest to left, 1 is farthest to right.
    int density = 10; // noise sources per unit resolution
    
    bool shouldUpdateGenerators = true; // updates generators and position to match resolution, freqIdx, and volumeGain
    bool shouldUpdateFilters = true;
    
    float MIN_FREQ = 20.0f;
    float MAX_FREQ = 20000.0f;
    
    float minFreq; // the actual calculated minFreq for this resolution
    float maxFreq; // the actual calculated maxFreq for this resolution
    float bottomFreq; // the relative minFreq for this range [minFreq, maxFreq)
    float topFreq; // the relative maxFreq for this range (minFreq, maxFreq]
    float bottom = 0.0f; // from [0, 1) relative position of bottomFreq between minFreq and maxFreq
    float top = 1.0f; // from (0, 1] - the relative position of topFreq between minFreq and maxFreq
};
