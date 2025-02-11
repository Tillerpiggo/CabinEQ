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
        lowPassFiltersLeft.push_back (juce::dsp::IIR::Filter<float>());
        lowPassFiltersRight.push_back (juce::dsp::IIR::Filter<float>());
        highPassFiltersLeft.push_back (juce::dsp::IIR::Filter<float>());
        highPassFiltersRight.push_back (juce::dsp::IIR::Filter<float>());
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
    if (freqIdx > 0)
    {
        for (int i = 0; i < order; ++i)
        {
            nextLeftSample = lowPassFiltersLeft[i].processSample (nextLeftSample);
            nextRightSample = lowPassFiltersRight[i].processSample (nextRightSample);
        }
    }
    
    if (freqIdx < resolution - 1)
    {
        for (int i = 0; i < order; ++i)
        {
            nextLeftSample = highPassFiltersLeft[i].processSample (nextLeftSample);
            nextRightSample = highPassFiltersRight[i].processSample (nextRightSample);
        }
    }
    
    // Reset the filters if needed
    if (snapToZeroCounter >= 1000)
    {
        for (int i = 0; i < order; ++i)
        {
            lowPassFiltersLeft[i].snapToZero();
            lowPassFiltersRight[i].snapToZero();
            highPassFiltersLeft[i].snapToZero();
            highPassFiltersRight[i].snapToZero();
        }
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
    return { nextLeftSample, nextRightSample };
}

void SquareGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    for (int i = 0; i < order; ++i)
    {
        lowPassFiltersLeft[i].prepare (spec);
        lowPassFiltersRight[i].prepare (spec);
        highPassFiltersLeft[i].prepare (spec);
        highPassFiltersRight[i].prepare (spec);
    }
}

void SquareGenerator::setResolution (int resolution)
{
    this->resolution = resolution;
    shouldUpdateGenerators = true;
}

void SquareGenerator::setCheckerboardCoords (int panIdx, int freqIdx)
{
    if (freqIdx >= resolution || freqIdx < 0)
    {
        std::cerr << "ERR: trying to set out of bounds freqIdx (freqIdx: " << freqIdx << ", resolution: " << resolution << ") in SquareGenerator::setFreqIdx" << std::endl;
        return;
    }
    
    if (panIdx >= resolution || panIdx < 0)
    {
        std::cerr << "ERR: trying to set out of bounds panIdx (panIdx: " << panIdx << ", resolution: " << resolution << ") in SquareGenerator::setPanIdx" << std::endl;
    }
    
    this->freqIdx = freqIdx;
    this->panIdx = panIdx;
    shouldUpdateGenerators = true;
}
//
//void SquareGenerator::setFreqIdx (int freqIdx)
//{
//    if (freqIdx >= resolution || freqIdx < 0)
//    {
//        std::cerr << "ERR: trying to set out of bounds freqIdx (freqIdx: " << freqIdx << ", resolution: " << resolution << ") in SquareGenerator::setFreqIdx" << std::endl;
//        return;
//    }
//    this->freqIdx = freqIdx;
//    updateGeneratorsIfNeeded();
//}
//
//void SquareGenerator::setPanIdx (int panIdx)
//{
//    if (panIdx >= resolution || panIdx < resolution)
//    {
//        std::cerr << "ERR: trying to set out of bounds panIdx (panIdx: " << panIdx << ", resolution: " << resolution << ") in SquareGenerator::setPanIdx" << std::endl;
//    }
//    this->panIdx = panIdx;
//    updateGeneratorsIfNeeded();
//}

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
        *lowPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highFreq);
        *lowPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, highFreq);
        *highPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowFreq);
        *highPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, lowFreq);
    }
    shouldUpdateGenerators = false;
    
    // Update the panning based on panIdx and resolution
    float panStep = 1.0f / (float) resolution;
    float startPan = panStep * (panIdx); // mapped to [0, 1]
    float endPan = panStep * (panIdx + 1.0f); // mapped to [0, 1]
    startPan = startPan * 2.0f - 1.0f; // map to [-1, 1]
    endPan = endPan * 2.0f - 1.0f; // map to [-1, 1]
    float panWidth = endPan - startPan;
    
//    std::cout << "panWidth: " << panWidth << std::endl;
    
    // Populate leftRightGains
    leftRightGains.clear();
    float interPanStep = panWidth / (float) density; // panning division within this one square
    for (int i = 0; i < density; ++i)
    {
        float interPan = startPan + ((float) i + 0.5f) * interPanStep;
        std::cout << "panIdx: " << panIdx << ", interpan: " << interPan << std::endl;
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
