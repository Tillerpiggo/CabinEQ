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
    for (auto& noiseGenerator : noiseGenerators)
    {
        noiseGenerator.prepare (spec);
    }
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

void MultiplePatternGenerator::addPattern (std::vector<bool> hits)
{
    NoiseGenerator newNoiseGenerator;
    newNoiseGenerator.setPattern (Pattern (hits));
}

void MultiplePatternGenerator::addPatterns (std::vector<std::vector<bool>> hitsVector)
{
    for (const auto& hits : hitsVector)
        addPattern (hits);
}
