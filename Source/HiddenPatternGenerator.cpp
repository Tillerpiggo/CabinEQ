/*
  ==============================================================================

    HiddenPatternGenerator.cpp
    Created: 7 Nov 2024 11:37:04am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "HiddenPatternGenerator.h"


HiddenPatternGenerator::HiddenPatternGenerator()
{
}

std::pair<float, float> HiddenPatternGenerator::getNextSample()
{
    if (isMuted || ! hiddenPattern.has_value())
        return { 0.0f, 0.0f };
    
    std::pair<float, float> nextSample = hiddenGenerator.getNextSample();
    
    for (auto& confoundingGenerator : confoundingGenerators)
    {
        auto confoundingSample = confoundingGenerator.getNextSample();
        nextSample.first += confoundingSample.first;
        nextSample.second += confoundingSample.second;
    }
    
    return nextSample;
}

void HiddenPatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    hiddenGenerator.prepare (spec);
    // prepare other generators as they are added
}

void HiddenPatternGenerator::setPattern (HiddenPattern hiddenPattern)
{
    // Update hidden sweep pattern
    this->hiddenPattern = hiddenPattern;
    hiddenGenerator.setSweepPattern (hiddenPattern.getHiddenSweepPattern (spec.sampleRate));
    hiddenGenerator.setBandwidth (hiddenBandwidth);
    
    // Add needed generators
    auto confoundingSweepPatterns = hiddenPattern.getConfoundingSweepPatterns (spec.sampleRate);
    int numToAdd = static_cast<int> (confoundingSweepPatterns.size()) - static_cast<int> (confoundingGenerators.size());
    for (int i = 0; i < numToAdd; ++i)
    {
        confoundingGenerators.push_back (NoiseSweepGenerator());
        confoundingGenerators[confoundingGenerators.size() - 1].prepare (spec);
    }
    
    // Set all sweep patterns
    for (int i = 0; i < confoundingGenerators.size(); ++i)
    {
        if (i < confoundingSweepPatterns.size())
        {
            confoundingGenerators[i].setSweepPattern (confoundingSweepPatterns[i]);
            confoundingGenerators[i].setBandwidth (confoundingBandwidth);
        }
        else
        {
            confoundingGenerators[i].mute();
        }
    }
    
}

void HiddenPatternGenerator::setSpeedFactor (float speedFactor)
{
    
}

void HiddenPatternGenerator::setHiddenBandwidth (float hiddenBandwidth)
{
    
}

void HiddenPatternGenerator::setConfoundingBandwidth (float confoundingBandwidth)
{
    
}

void HiddenPatternGenerator::mute()
{
    
}

std::optional<float> HiddenPatternGenerator::getCurrPlayingFreq()
{
    
}

void HiddenPatternGenerator::prepareGeneratorsWithHiddenPattern()
{
}
