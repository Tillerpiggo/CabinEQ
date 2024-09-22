/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 12 Sep 2024 11:10:34pm
    Author:  Tyler Gee

  ==============================================================================
*/



#include "SpatialNoiseGenerator.h"
#include <algorithm>

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate(44100.0f),
      centralFrequency(1000.0f),
      bandwidth(1.0f),
      bwHeadFactor(1.0f),
      bwTailFactor(1.0f)
{
    frequencies.resize(numSinWaves);
    amplitudes.resize(numSinWaves);
    phaseIndices.resize(numSinWaves);
    phaseIncrements.resize(numSinWaves);
    tableSizes.resize(numSinWaves);
    sineWaveTables.resize(numSinWaves);

    // Initialize frequencies, phase indices, and tables
    setSampleRate(sampleRate);
    precomputeSineTables(); // Precompute sine waves
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;

    // Calculate frequencies logarithmically spaced between 20Hz and 20kHz
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;

    for (int i = 0; i < numSinWaves; ++i)
    {
        float t = static_cast<float>(i) / (numSinWaves - 1);
        frequencies[i] = minFreq * std::pow(maxFreq / minFreq, t);

        // Update table sizes based on frequencies
        tableSizes[i] = static_cast<int>(sampleRate / frequencies[i]);
        tableSizes[i] = std::max(tableSizes[i], 1); // Ensure at least one sample

        // Update phase increments
        phaseIncrements[i] = frequencies[i] / sampleRate;

        // Initialize phase indices with random starting positions
        phaseIndices[i] = random.nextFloat() * tableSizes[i];
    }

    precomputeSineTables();
    updateAmplitudes();
}

void SpatialNoiseGenerator::precomputeSineTables()
{
    // Precompute one period of each sine wave
    for (int i = 0; i < numSinWaves; ++i)
    {
        int tableSize = tableSizes[i];
        sineWaveTables[i].resize(tableSize);

        for (int j = 0; j < tableSize; ++j)
        {
            float angle = (static_cast<float>(j) / tableSize) * juce::MathConstants<float>::twoPi;
            sineWaveTables[i][j] = std::sin(angle);
        }
    }
}

void SpatialNoiseGenerator::updateAmplitudes()
{
    for (int i = 0; i < numSinWaves; ++i)
    {
        float freq = frequencies[i];

        // Pink noise amplitude weighting
        amplitudes[i] = juce::Decibels::decibelsToGain(
            amplCurve.valueAtFrequency(freq) +
            -4.5f * std::log2(freq / 1000.0f));

        // Bandpass filtering
        float logDistance = std::abs(std::log2(freq / centralFrequency));
        if (freq > centralFrequency)
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;

        float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
        amplitudes[i] *= bandpassGain;
    }
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample()
{
    float sample = 0.0f;

    for (int i = 0; i < numSinWaves; ++i)
    {
        int tableSize = tableSizes[i];
        int index = static_cast<int>(phaseIndices[i]) % tableSize;

        sample += sineWaveTables[i][index] * amplitudes[i];

        // Increment phase index
        phaseIndices[i] += phaseIncrements[i] * tableSize;
        if (phaseIndices[i] >= tableSize)
            phaseIndices[i] -= tableSize;
    }

    sample /= numSinWaves;
    sample *= 20.0f; // Adjust the overall amplitude as needed

    return std::make_pair(sample, sample); // Mono output
}

void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
{
    this->amplCurve = amplCurve;
    updateAmplitudes();
}

void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
    this->bwHeadFactor = bwHeadFactor;
    this->bwTailFactor = bwTailFactor;

    updateAmplitudes();
}

/*
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
    float sample = 0.0f;

    for (int j = 0; j < numSinWaves; ++j)
    {
        phases[j] += phaseIncrements[j];
        if (phases[j] >= juce::MathConstants<float>::twoPi)
            phases[j] -= juce::MathConstants<float>::twoPi;

        sample += sineTable.get(phases[j]) * amplitudes[j];
    }

    sample /= numSinWaves;
    sample *= 50;

    return std::make_pair(sample, sample);
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;
    
    for (int i = 0; i < numSinWaves; ++i)
    {
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        float logMin = std::log10(minFreq);
        float logMax = std::log10(maxFreq);
        
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
            amplitudes[i] = 0;
            continue;
        }

        amplitudes[i] = juce::Decibels::decibelsToGain(amplCurve.valueAtFrequency(frequencies[i]) +
                                                       -4.5 * std::log2(frequencies[i] / 1000.0f)); // Pink noise
        float freq = frequencies[i];
        float logDistance = std::abs(std::log2(freq / centralFrequency));
        if (freq > centralFrequency) // Long head
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;

        float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
        amplitudes[i] *= bandpassGain;
    }
}
*/

/*
#include "SpatialNoiseGenerator.h"
#include <algorithm>

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate(44100.0f),
      centralFrequency(1000.0f),
      bandwidth(1.0f),
      bwHeadFactor(1.0f),
      bwTailFactor(1.0f)
{
    frequencies.resize(numSinWaves);
    amplitudes.resize(numSinWaves);
    phaseIndices.resize(numSinWaves);
    phaseIncrements.resize(numSinWaves);
    tableSizes.resize(numSinWaves);
    sineWaveTables.resize(numSinWaves);

    // Initialize frequencies, phase indices, and tables
    setSampleRate(sampleRate);
    precomputeSineTables(); // Precompute sine waves
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;

    // Calculate frequencies logarithmically spaced between 20Hz and 20kHz
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;

    for (int i = 0; i < numSinWaves; ++i)
    {
        float t = static_cast<float>(i) / (numSinWaves - 1);
        frequencies[i] = minFreq * std::pow(maxFreq / minFreq, t);

        // Update table sizes based on frequencies
        tableSizes[i] = static_cast<int>(sampleRate / frequencies[i]);
        tableSizes[i] = std::max(tableSizes[i], 1); // Ensure at least one sample

        // Update phase increments
        phaseIncrements[i] = frequencies[i] / sampleRate;

        // Initialize phase indices with random starting positions
        phaseIndices[i] = random.nextFloat() * tableSizes[i];
    }

    precomputeSineTables();
    updateAmplitudes();
}

void SpatialNoiseGenerator::precomputeSineTables()
{
    // Precompute one period of each sine wave
    for (int i = 0; i < numSinWaves; ++i)
    {
        int tableSize = tableSizes[i];
        sineWaveTables[i].resize(tableSize);

        for (int j = 0; j < tableSize; ++j)
        {
            float angle = (static_cast<float>(j) / tableSize) * juce::MathConstants<float>::twoPi;
            sineWaveTables[i][j] = std::sin(angle);
        }
    }
}

void SpatialNoiseGenerator::updateAmplitudes()
{
    for (int i = 0; i < numSinWaves; ++i)
    {
        float freq = frequencies[i];

        // Pink noise amplitude weighting
        amplitudes[i] = juce::Decibels::decibelsToGain(
            amplCurve.valueAtFrequency(freq) +
            -4.5f * std::log2(freq / 1000.0f));

        // Bandpass filtering
        float logDistance = std::abs(std::log2(freq / centralFrequency));
        if (freq > centralFrequency)
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;

        float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
        amplitudes[i] *= bandpassGain;
    }
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample()
{
    float sample = 0.0f;

    for (int i = 0; i < numSinWaves; ++i)
    {
        int tableSize = tableSizes[i];
        int index = static_cast<int>(phaseIndices[i]) % tableSize;

        sample += sineWaveTables[i][index] * amplitudes[i];

        // Increment phase index
        phaseIndices[i] += phaseIncrements[i] * tableSize;
        if (phaseIndices[i] >= tableSize)
            phaseIndices[i] -= tableSize;
    }

    sample /= numSinWaves;
    sample *= 50.0f; // Adjust the overall amplitude as needed

    return std::make_pair(sample, sample); // Mono output
}

void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
{
    this->amplCurve = amplCurve;
    updateAmplitudes();
}

void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
    this->bwHeadFactor = bwHeadFactor;
    this->bwTailFactor = bwTailFactor;

    updateAmplitudes();
}

*/
