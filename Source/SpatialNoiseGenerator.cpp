/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialNoiseGenerator.h"

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate(44100.0f), // Set a default sample rate
      centralFrequency(0),
      bandwidth(0)
{
    frequencies.resize(numSinWaves);
    amplitudes.resize(numSinWaves);
    phases.resize(numSinWaves);
    phaseIncrements.resize(numSinWaves, 0.0f);

    juce::Random random; // JUCE random number generator

    for (int i = 0; i < numSinWaves; ++i)
    {
        // Assign random phase between 0 and 2π
        phases[i] = random.nextFloat() * (2.0f * juce::MathConstants<float>::pi);
    }
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample()
{
    float leftSample = 0.0f;
    float rightSample = 0.0f;

    for (int j = 0; j < numSinWaves; ++j)
    {
        phases[j] += phaseIncrements[j];
        if (phases[j] >= juce::MathConstants<float>::twoPi)
            phases[j] -= juce::MathConstants<float>::twoPi;
        
        float sinVal = sineTable.get(phases[j]);
        std::pair<float, float> ampl = amplitudes[j];
        leftSample += sinVal * ampl.first;
        rightSample += sinVal * ampl.second;
    }

    leftSample /= numSinWaves;
    rightSample /= numSinWaves;
    leftSample *= 50;
    rightSample *= 50;

    return { leftSample, rightSample };
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;
    
    juce::Random random;
    
    for (int i = 0; i < numSinWaves; ++i)
    {
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        float logMin = std::log10(minFreq);
        float logMax = std::log10(maxFreq);
        
        minFreq += random.nextFloat() * 5.0f;
        
        // Calculate the logarithmic step
        float logStep = (logMax - logMin) / (numSinWaves - 1);
        
        // Calculate the frequency for each wave
        frequencies[i] = std::pow(10.0f, logMin + i * logStep);
        phaseIncrements[i] = frequencies[i] * (2.0f * juce::MathConstants<float>::pi) / sampleRate;
    }
}


void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
{
    this->amplCurve = amplCurve;
}

void SpatialNoiseGenerator::setPanCurve (Curve panCurve)
{
    this->panCurve = panCurve;
}

void SpatialNoiseGenerator::setPhaseCurve (Curve phaseCurve)
{
    this->phaseCurve = phaseCurve;
}

void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
    this->bwHeadFactor = bwHeadFactor;
    this->bwTailFactor = bwTailFactor;

    for (int i = 0; i < numSinWaves; ++i)
    {
        if (bandwidth == 0)
        {
            amplitudes[i] = { 0, 0 };
            continue;
        }
        
        float amplVal = amplCurve.valueAtFrequency (frequencies[i]);
//        float panVal = panCurve.valueAtFrequency (frequencies[i]);
//        float amplVal = 0;
        float panVal = 0;
        
        float cabinNoiseDropoff = -4.5 * std::log2 (frequencies[i] / 1000.0f);
        float leftAmpl = juce::Decibels::decibelsToGain (amplVal - 0.5 * panVal + pinkNoiseDropoff);
        float rightAmpl = juce::Decibels::decibelsToGain (amplVal + 0.5 * panVal + pinkNoiseDropoff);
        amplitudes[i] = { leftAmpl, rightAmpl };
        float freq = frequencies[i];
        float logDistance = std::abs(std::log2(freq / centralFrequency));
        if (freq > centralFrequency) // Long head
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;
        
        float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
        float slope = -24.0f / bandwidth;
        float bandpassDBChange = logDistance * slope;
//        bandpassGain *= juce::Decibels::decibelsToGain (bandpassDBChange);
        amplitudes[i].first *= bandpassGain;
        amplitudes[i].second *= bandpassGain;
    }
}

