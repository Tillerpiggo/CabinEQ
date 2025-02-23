/*
  ==============================================================================

    RowPlayer.h
    Created: 23 Feb 2025 10:25:58am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class RowPlayer
{
public:
    RowPlayer();
    
    std::pair<float, float> getNextSample();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setFrequency (float centerFreqHz);
    void setBandwidth (float bandwidthOctaves);
    void setMinAndMaxFreqs (float minFreqHz, float maxFreqHz);
    void setPlayingIdx (int playingIdx);
    
private:
    void updateFiltersIfNeeded();
    
    float sampleRate;
    float totalGain = 1.0f;
    float centerFreq;
    float bandwidth;
    std::vector<std::pair<float, float>> leftRightGains;
    
    
    juce::Random random;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFiltersLeft;
    std::vector<juce::dsp::IIR::Filter<float>> lowPassFiltersRight;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFiltersLeft;
    std::vector<juce::dsp::IIR::Filter<float>> highPassFiltersRight;
    int order = 4;
    int snapToZeroCounter = 0;
    int density = 5; // noise sources per row

    int playingIdx = -1;

    float MIN_FREQ = 20.0f;
    float MAX_FREQ = 20000.0f;

    bool shouldUpdateGenerators = true;
    bool shouldUpdateFilters = true;
};
