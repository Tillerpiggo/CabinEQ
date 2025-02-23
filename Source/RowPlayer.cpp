/*
  ==============================================================================

    RowPlayer.cpp
    Created: 23 Feb 2025 10:25:58am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "RowPlayer.h"

RowPlayer::RowPlayer()
{
    // Initialize filters
    for (int i = 0; i < order; ++i)
    {
        lowPassFiltersLeft.push_back(juce::dsp::IIR::Filter<float>());
        lowPassFiltersRight.push_back(juce::dsp::IIR::Filter<float>());
        highPassFiltersLeft.push_back(juce::dsp::IIR::Filter<float>());
        highPassFiltersRight.push_back(juce::dsp::IIR::Filter<float>());
    }
}

std::pair<float, float> RowPlayer::getNextSample()
{
    updateFiltersIfNeeded();

    // Get the (white) noise samples by adding white noise
    float leftSample = 0.0f;
    float rightSample = 0.0f;

    // Generate and sum noise sources
    for (int i = 0; i < density; ++i)
    {
        // If playing a specific index, don't play other indices
        if (playingIdx != -1 && i != playingIdx)
            continue;

        float noise = (random.nextFloat() * 2.0f - 1.0f) * totalGain;
        leftSample += noise * leftRightGains[i].first;
        rightSample += noise * leftRightGains[i].second;
    }

    // Apply filters
    for (int i = 0; i < order; ++i)
    {
        leftSample = lowPassFiltersLeft[i].processSample(leftSample);
        rightSample = lowPassFiltersRight[i].processSample(rightSample);
        leftSample = highPassFiltersLeft[i].processSample(leftSample);
        rightSample = highPassFiltersRight[i].processSample(rightSample);
    }

    // Snap to zero if very small
    if (snapToZeroCounter >= 10000)
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
    
    return { leftSample, rightSample };
}

void RowPlayer::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (int i = 0; i < order; ++i)
    {
        lowPassFiltersLeft[i].prepare(spec);
        lowPassFiltersRight[i].prepare(spec);
        highPassFiltersLeft[i].prepare(spec);
        highPassFiltersRight[i].prepare(spec);
    }
    shouldUpdateFilters = true;
}

void RowPlayer::setFrequency(float centerFreqHz)
{
    this->centerFreq = centerFreqHz;
    shouldUpdateFilters = true;
}

void RowPlayer::setBandwidth(float bandwidthOctaves)
{
    this->bandwidth = bandwidthOctaves;
    shouldUpdateFilters = true;
}

void RowPlayer::setMinAndMaxFreqs(float minFreqHz, float maxFreqHz)
{
    MIN_FREQ = minFreqHz;
    MAX_FREQ = maxFreqHz;
    shouldUpdateFilters = true;
}

void RowPlayer::setPlayingIdx(int playingIdx)
{
    this->playingIdx = playingIdx;
}

void RowPlayer::updateFiltersIfNeeded()
{
    if (!shouldUpdateFilters)
        return;

    // Calculate filter frequencies
    float lowFreq = std::max(centerFreq * std::pow(2.0f, -bandwidth), MIN_FREQ);
    float highFreq = std::min(centerFreq * std::pow(2.0f, bandwidth), MAX_FREQ);

    // Update all filter coefficients
    for (int i = 0; i < order; ++i)
    {
        auto lowPassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, highFreq);
        auto highPassCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowFreq);

        lowPassFiltersLeft[i].coefficients = lowPassCoeffs;
        lowPassFiltersRight[i].coefficients = lowPassCoeffs;
        highPassFiltersLeft[i].coefficients = highPassCoeffs;
        highPassFiltersRight[i].coefficients = highPassCoeffs;
    }

    // Figure out the panning based on density, where its evenly spaced across
    float startPan = -1;
    float endPan = 1;
    float panWidth = endPan - startPan;

    leftRightGains.clear();
    float panStep = panWidth / ((float) density - 1.0f); // panning division within this one square
    for (int i = 0; i < density; ++i)
    {
        float interPan = startPan + i * panStep;
        float angle = (interPan + 1.0f) * M_PI / 4.0f; // map pan from [-1, 1] to angle [0, π/2]
        float leftGain = std::cos (angle);
        float rightGain = std::sin (angle);
        leftRightGains.push_back({ leftGain, rightGain });
    }


    shouldUpdateFilters = false;
}
