/*
  ==============================================================================

    MultiplePatternGenerator.cpp
    Created: 6 Nov 2024 1:08:19am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MultiplePatternGenerator.h"

MultiplePatternGenerator::MultiplePatternGenerator()
{
}


std::pair<float, float> MultiplePatternGenerator::getNextSample()
{
    std::pair<float, float> nextSample { 0.0f, 0.0f };
    
    if (isMuted)
        return nextSample;
    
    for (auto& noiseGenerator : noiseGenerators)
    {
        auto noiseGeneratorSample = noiseGenerator.getNextSample();
        nextSample.first += noiseGeneratorSample.first;
        nextSample.second += noiseGeneratorSample.second;
    }
    
    return nextSample;
}

void MultiplePatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
}

void MultiplePatternGenerator::setSpeedFactor (float speedFactor)
{
    // TODO: Implement
}

void MultiplePatternGenerator::setFreqFactor (float freqFactor)
{
    // TODO: Implement
}

void MultiplePatternGenerator::mute()
{
    isMuted = true;
}


void MultiplePatternGenerator::setPattern (FauxMusicPattern fauxMusicPattern)
{
    // For all existing generators, simply set their pattern
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        noiseGenerators[i].setPattern (fauxMusicPattern.getPatterns()[i]);
        noiseGenerators[i].setFrequencyRange (fauxMusicPattern.getFreqRanges()[i]);
    }
    
    // For any new needed generators, add them, prepare them, and then set their pattern
    int numToAdd = static_cast<int> (fauxMusicPattern.getNumPatterns()) - static_cast<int> (noiseGenerators.size());
    int numNoiseGenerators = static_cast<int> (noiseGenerators.size());
    for (int i = 0; i < numToAdd; ++i)
    {
        int idx = i + numNoiseGenerators;
        noiseGenerators.push_back (NoiseGenerator());
        noiseGenerators[idx].setPattern (fauxMusicPattern.getPatterns()[i]);
        noiseGenerators[idx].setFrequencyRange (fauxMusicPattern.getFreqRanges()[i]);
        noiseGenerators[idx].prepare (spec);
    }
    
    isMuted = false;
}
