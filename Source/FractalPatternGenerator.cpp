/*
  ==============================================================================

    FractalPatternGenerator.cpp
    Created: 9 Nov 2024 7:32:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FractalPatternGenerator.h"

FractalPatternGenerator::FractalPatternGenerator()
{
}

std::pair<float, float> FractalPatternGenerator::getNextSample()
{
    if (isMuted || ! fractalPattern.has_value())
        return { 0.0f, 0.0f };
    
    std::pair<float, float> nextSample = hiddenGenerator.getNextSample();
    
    // Calculate and aplpy gain
    if (sampleIdx >= hitDurationInSamples)
    {
        sampleIdx = remainder (sampleIdx, hitDurationInSamples);
        hitIdx++;
        if (hitIdx >= hits.size())
            hitIdx = 0;
    }
    
    float gain = gainEnvelope.gainAtSample (sampleIdx, hitDurationInSamples);
    
    nextSample.first *= gain;
    nextSample.second *= gain;
    
    if (hits[hitIdx] == false)
        nextSample = { 0.0f, 0.0f };
    
    sampleIdx++;
    
    // Add confounding noises
    for (auto& confoundingGenerator : confoundingGenerators)
    {
        auto confoundingSample = confoundingGenerator.getNextSample();
        nextSample.first += confoundingSample.first;
        nextSample.second += confoundingSample.second;
    }
    
    updateGeneratorBandpassFilters();
    currTime += timeIncrement * speedFactor;
    
    return nextSample;
}

void FractalPatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    hiddenGenerator.prepare (spec);
    // confounding generators will get prpeared as they are created, and must be added *after* prepare is called
}

void FractalPatternGenerator::setPattern (FractalPattern fractalPattern)
{
    this->fractalPattern = fractalPattern;
    updateNoiseGenerators();
    
    isMuted = false;
}

void FractalPatternGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void FractalPatternGenerator::setBandwidth (float bandwidth)
{
    this->bandwidth = bandwidth;
}

void FractalPatternGenerator::setNumConfoundingGenerators (int numConfoundingGenerators)
{
    this->numConfoundingGenerators = numConfoundingGenerators;
    
    offsets.clear();
    float fraction = 1.0f / static_cast<float> (numConfoundingGenerators + 1);
    for (int i = 0; i < numConfoundingGenerators; ++i)
    {
        offsets.push_back (static_cast<float> (i + 1) * fraction);
    }
}

void FractalPatternGenerator::mute()
{
    isMuted = true;
}

std::optional<float> FractalPatternGenerator::getCurrPlayingFreq()
{
    return std::nullopt;
}

void FractalPatternGenerator::updateBandwidth()
{
    hiddenGenerator.setBandwidth (bandwidth);
    
    for (auto& confoundingGenerator : confoundingGenerators)
        confoundingGenerator.setBandwidth (bandwidth);
}

void FractalPatternGenerator::updateNoiseGenerators()
{
    // Make sure we have numConfoundingGenerators generators
    int numToAdd = numConfoundingGenerators - static_cast<int> (confoundingGenerators.size());
    for (int i = 0; i < numToAdd; ++i)
    {
        confoundingGenerators.push_back (NoiseGenerator());
        confoundingGenerators[confoundingGenerators.size() - 1].prepare (spec);
    }
    
    updateBandwidth();
}

void FractalPatternGenerator::updateGeneratorBandpassFilters()
{
    // Get the hidden generator value
    auto [hiddenFreq, hiddenPan] = fractalPattern->getFrequencyAndPanAtTime (currTime);
    hiddenGenerator.setBandpass (hiddenFreq);
    hiddenGenerator.setPan (hiddenPan);
    
    for (int i = 0; i < confoundingGenerators.size(); ++i)
    {
        auto [freq, pan] = fractalPattern->getFrequencyAndPanAtTime (remainder (currTime + offsets[i], 1.0f));
        confoundingGenerators[i].setBandpass (freq);
        confoundingGenerators[i].setPan (pan);
    }
}
