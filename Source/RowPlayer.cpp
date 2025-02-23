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
        float noise = (random.nextFloat() * 2.0f - 1.0f) * totalGain / (float) density;
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

void RowPlayer::updateFiltersIfNeeded()
{
    if (!shouldUpdateFilters)
        return;

    // Calculate filter frequencies
    float lowFreq = centerFreq * std::pow(2.0f, -bandwidth / 2.0f);
    float highFreq = centerFreq * std::pow(2.0f, bandwidth / 2.0f);

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

    shouldUpdateFilters = false;
}
