/*
  ==============================================================================

    SquareGenerator.cpp
    Created: 10 Feb 2025 3:15:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SquareGenerator.h"

SquareGenerator::SquareGenerator()
{
    for (int i = 0; i < order; ++i)
    {
        lowPassFilters.push_back (juce::dsp::IIR::Filter<float>());
        highPassFilters.push_back (juce::dsp::IIR::Filter<float>());
    }
}

std::pair<float, float> SquareGenerator::getNextSample()
{
    updateGeneratorsIfNeeded();
    
    // Get the pink noise sample by adding pink noises
    float nextLeftSample = 0.0f;
    float nextRightSample = 0.0f;
    for (int i = 0; i < pinkNoises.size(); ++i)
    {
        auto [leftGain, rightGain] = leftRightGains[i];
        float pinkNoiseSample = pinkNoises[i].generate() * 10.0f * totalGain / (float) density;
        nextLeftSample += pinkNoiseSample * leftGain;
        nextRightSample += pinkNoiseSample * rightGain;
    }
    
    // Filter the pink noise
    for (int i = 0; i < order; ++i)
    {
        pinkNoiseSample = lowPassFilters[i].processSample (nextLeftSample);
    }
    
    // Reset the filters if needed
    if (snapToZeroCounter >= 1000)
    {
        for (int i = 0; i < order; ++i)
        {
            lowPassFilters[i].snapToZero();
            highPassFilters[i].snapToZero();
        }
        snapToZeroCounter = 0;
    }
    
    
}

void SquareGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    
}

void SquareGenerator::setResolution (int resolution)
{
    
}

void SquareGenerator::setFreqIdx (int freqIdx)
{
    if (freqIdx >= resolution || freqIdx < 0)
    {
        std::cerr << "ERR: trying to set out of bounds freqIdx (freqIdx: " << freqIdx << ", resolution: " << resolution << ") in SquareGenerator::setFreqIdx" << std::endl;
        return;
    }
    this->freqIdx = freqIdx;
}

void SquareGenerator::setPanIdx (int panIdx)
{
    if (panIdx >= resolution || panIdx < resolution)
    {
        std::cerr << "ERR: trying to set out of bounds panIdx (panIdx: " << panIdx << ", resolution: " << resolution << ") in SquareGenerator::setPanIdx" << std::endl;
    }
}

void SquareGenerator::setVolumeGain (float volumeGain)
{
    this->totalGain = volumeGain;
}

void SquareGenerator::updateGeneratorsIfNeeded()
{
    if (! shouldUpdateGenerators)
        return;
    
    // Calculate low and high freq based off of freqIdx and resolution
    float logMin = std::log2 (MIN_FREQ);
    float logMax = std::log2 (MAX_FREQ);
    float freqStep = (logMax - logMin) / resolution;
    
    float lowFreq = std::pow (2.0f, logMin + freqIdx * freqStep);
    float highFreq = std::pow (2.0f, logMin + (freqIdx + 1.0f) * freqStep);
    
    // Compute low and high pass filters to match lowFreq/highFreq bounds
    for (int i = 0; i < order; ++i)
    {
        *lowPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highFreq);
        *highPassFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowFreq);
    }
    shouldUpdateGenerators = false;
    
    // Update the panning based on panIdx and resolution
    float panStep = 1.0f / resolution;
    float startPan = panStep * (panIdx); // mapped to [0, 1]
    float endPan = panStep * (panIdx); // mapped to [0, 1]
    startPan = startPan * 2.0f - 1.0f; // map to [-1, 1]
    endPan = endPan * 2.0f - 1.0f; // map to [-1, 1]
    float panWidth = endPan - startPan;
    
    // Populate leftRightGains
    leftRightGains.clear();
    float interPanStep = panWidth / density; // panning division within this one square
    for (int i = 0; i < density; ++i)
    {
        float interPan = (i + 0.5f) * interPanStep;
        float angle = (interPan + 1.0f) * M_PI / 4.0f; // map pan from [-1, 1] to angle [0, π/2]
        float leftGain = std::cos (angle);
        float rightGain = std::sin (angle);
        leftRightGains.push_back ({ leftGain, rightGain });
    }
    
    // Get correct # of pink noise generators
    pinkNoises.clear();
    for (int i = 0; i < density; i++)
    {
        pinkNoises.push_back (PinkNoise());
    }
}
