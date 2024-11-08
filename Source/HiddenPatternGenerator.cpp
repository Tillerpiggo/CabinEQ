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
    
    // Calculate and apply gain
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
    
    sampleIdx++;
    
    if (hits[hitIdx] == false)
        nextSample = { 0.0f, 0.0f };
    
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
}

void HiddenPatternGenerator::setPattern (HiddenPattern hiddenPattern)
{
    // Update hidden sweep pattern
    this->hiddenPattern = hiddenPattern;
    hiddenGenerator.setSweepPattern (hiddenPattern.getHiddenSweepPattern (spec.sampleRate));
    
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
    
    isMuted = false;
    
    this->hiddenBandwidth = hiddenPattern.getHiddenBandwidth();
    this->confoundingBandwidth = hiddenPattern.getConfoundingBandwidth();
    updateBandwidthsAndSpeedFactors();
}

void HiddenPatternGenerator::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
    updateBandwidthsAndSpeedFactors();
}

void HiddenPatternGenerator::setHiddenBandwidth (float hiddenBandwidth)
{
    this->hiddenBandwidth = hiddenBandwidth;
    updateBandwidthsAndSpeedFactors();
}

void HiddenPatternGenerator::setConfoundingBandwidth (float confoundingBandwidth)
{
    this->confoundingBandwidth = confoundingBandwidth;
    updateBandwidthsAndSpeedFactors();
}

void HiddenPatternGenerator::setConfoundingBandwidthMultiplier (float confoundingBandwidthMultiplier)
{
    this->confoundingBandwidthMultiplier = confoundingBandwidthMultiplier;
    updateBandwidthsAndSpeedFactors();
}

void HiddenPatternGenerator::updateBandwidthsAndSpeedFactors()
{
    hiddenGenerator.setBandwidth (hiddenBandwidth);
    hiddenGenerator.setSpeedFactor (speedFactor);
    
    for (auto& confoundingGenerator : confoundingGenerators)
    {
        confoundingGenerator.setBandwidth (confoundingBandwidth * confoundingBandwidthMultiplier);
    }
}

void HiddenPatternGenerator::mute()
{
    isMuted = true;
}

void HiddenPatternGenerator::setIsFrozen (bool isFrozen)
{
    this->isFrozen = isFrozen;
    this->hiddenGenerator.setIsFrozen (isFrozen);
}

std::optional<float> HiddenPatternGenerator::getCurrPlayingFreq()
{
    return hiddenGenerator.getCurrPlayingFreq();
}
