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
    updateFiltersIfNeeded();
    
    // Get the pink noise sample by adding pink noises
    float nextLeftSample = 0.0f;
    float nextRightSample = 0.0f;
    for (int i = 0; i < leftRightGains.size(); ++i)
    {
        auto [leftGain, rightGain] = leftRightGains[i];
        float pinkNoiseSample = random.nextFloat() * totalGain * 10.0f / (float) density;//pinkNoises[i].generate() * 10.0f * totalGain / (float) density;
        nextLeftSample += pinkNoiseSample * leftGain;
        nextRightSample += pinkNoiseSample * rightGain;
    }
    
    if (currRampSample < rampSamples)
    {
        float MIN_DB = -120.0f;
        float rampPercent = (float) currRampSample / (float) rampSamples;
        float volDB = MIN_DB * (1.0f - rampPercent);
        
        float gain = juce::Decibels::decibelsToGain (volDB);
        nextLeftSample *= gain;
        nextRightSample *= gain;
        currRampSample++;
//        std::cout << "gain: " << gain << std::endl;
    }
    
    // Filter the pink noise
    if (freqIdx < numRows - 1)
    {
        for (int i = 0; i < order; ++i)
        {
            nextLeftSample = lowPassFiltersLeft[i].processSample (nextLeftSample);
            nextRightSample = lowPassFiltersRight[i].processSample (nextRightSample);
        }
    }
    
    for (int i = 0; i < order; ++i)
    {
        nextLeftSample = highPassFiltersLeft[i].processSample (nextLeftSample);
        nextRightSample = highPassFiltersRight[i].processSample (nextRightSample);
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

void SquareGenerator::setGridDimensions (int numRows, int numCols)
{
    this->numRows = numRows;
    this->numCols = numCols;
    shouldUpdateGenerators = true;
    shouldUpdateFilters = true;
    currRampSample = 0;
}

void SquareGenerator::setCheckerboardCoords (int freqIdx, int panIdx)
{
    if (freqIdx >= numRows || freqIdx < 0)
    {
        std::cerr << "ERR: trying to set out of bounds freqIdx (freqIdx: " << freqIdx << ", numRows: " << numRows << ") in SquareGenerator::setFreqIdx" << std::endl;
        return;
    }
    
    if (panIdx >= numCols || panIdx < 0)
    {
        std::cerr << "ERR: trying to set out of bounds panIdx (panIdx: " << panIdx << ", numCols: " << numCols << ") in SquareGenerator::setPanIdx" << std::endl;
    }
    
    this->freqIdx = freqIdx;
    this->panIdx = panIdx;
    shouldUpdateGenerators = true;
    shouldUpdateFilters = true;
    currRampSample = 0;
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

void SquareGenerator::setMinFreq (float minFreq)
{
    this->MIN_FREQ = minFreq;
    shouldUpdateGenerators = true;
    shouldUpdateFilters = true;
}

void SquareGenerator::setBandpassRange (float bottom, float top)
{
    // Make sure bottom/top are in bounds
    if (bottom < 0 || bottom > 1)
    {
        std::cerr << "setBandpassRange in SquareGenerator with bottom out of range (bottom = " << bottom << ")";
        return;
    }
    if (top < 0 || top > 1)
    {
        std::cerr << "setBandpassRange in SquareGenerator with top out of range (top = " << top << ")";
        return;
    }
    if (bottom >= top)
    {
        std::cerr << "setBandpassRange in SquareGenerator with bottom greater than top (bottom = " << bottom << ", top = " << top << ")" << std::endl;
        return;
    }
    
    this->bottom = bottom;
    this->top = top;
    shouldUpdateFilters = true;
}

void SquareGenerator::updateFiltersIfNeeded()
{
    if (! shouldUpdateFilters)
        return;
    
    // Logarithmically interpolate the correct bottom/top
    float logMinFreq = log2 (minFreq);
    float logMaxFreq = log2 (maxFreq);
    float range = logMaxFreq - logMinFreq;
    bottomFreq = pow (2.0f, logMinFreq + range * bottom);
    topFreq = pow (2.0f, logMinFreq + range * top);
    
    // Compute low and high pass filters to match lowFreq/highFreq bounds
    for (int i = 0; i < order; ++i)
    {
        *lowPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, topFreq);
        *lowPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, topFreq);
        *highPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, bottomFreq);
        *highPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, bottomFreq);
    }
    
    shouldUpdateFilters = false;
}

void SquareGenerator::updateGeneratorsIfNeeded()
{
    if (! shouldUpdateGenerators)
        return;
    
    // Calculate low and high freq based off of freqIdx and resolution
    float logMin = std::log2 (MIN_FREQ);
    float logMax = std::log2 (MAX_FREQ);
    float freqStep = (logMax - logMin) / (float) numRows;
    
    float lowFreq = std::pow (2.0f, logMin + freqIdx * freqStep);
    float highFreq = std::pow (2.0f, logMin + (freqIdx + 1.0f) * freqStep);
    
    minFreq = lowFreq;
    maxFreq = highFreq;
    
//    // Compute low and high pass filters to match lowFreq/highFreq bounds
//    for (int i = 0; i < order; ++i)
//    {
//        *lowPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, topFreq);
//        *lowPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, topFreq);
//        *highPassFiltersLeft[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, bottomFreq);
//        *highPassFiltersRight[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, bottomFreq);
//    }
    
    // Update the panning based on panIdx and resolution
    float spacing = 0.1f; // between each square
    float panStep = (1.0f - spacing * (numCols - 1)) / (float) numCols;
    float startPan = panStep * (panIdx); // mapped to [0, 1]
    float endPan = panStep * (panIdx + 1); // mapped to [0, 1]
    startPan += spacing * (panIdx);
    endPan += spacing * (panIdx);
    startPan = startPan * 2.0f - 1.0f; // map to [-1, 1]
    endPan = endPan * 2.0f - 1.0f; // map to [-1, 1]
    float panWidth = endPan - startPan;
    
    std::cout << "startPan: " << startPan << ", endPan: " << endPan << std::endl;
    
    // Populate leftRightGains
    leftRightGains.clear();
    float interPanStep = panWidth / ((float) density - 1.0f); // panning division within this one square
    for (int i = 0; i < density; ++i)
    {
        float interPan = startPan + i * interPanStep;
        float angle = (interPan + 1.0f) * M_PI / 4.0f; // map pan from [-1, 1] to angle [0, π/2]
        float leftGain = std::cos (angle);
        float rightGain = std::sin (angle);
        leftRightGains.push_back ({ leftGain, rightGain });
    }
    
    shouldUpdateGenerators = false;
    currRampSample = 0;
}
