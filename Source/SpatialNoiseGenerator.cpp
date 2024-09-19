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
    phases.resize(numSinWaves, 0.0f);          // Initialize phases
    phaseIncrements.resize(numSinWaves, 0.0f); // Initialize phase increments
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
        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq)); // Generate random frequency
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
                                                       -3.0f * std::log2(frequencies[i] / 1000.0f)); // Pink noise
        float freq = frequencies[i];
        float logDistance = std::abs(std::log2(freq / centralFrequency));
        if (freq > centralFrequency) // Long head
            logDistance *= bwHeadFactor;
        else
            logDistance *= bwTailFactor;

        logDistance *= logDistance;
        float logRatio = logDistance / bandwidth;

        float bandpassGain = std::exp(-5.0f * logRatio);
        amplitudes[i] *= bandpassGain;
    }
}

//#include "SpatialNoiseGenerator.h"
//
//SpatialNoiseGenerator::SpatialNoiseGenerator()
//    : sampleRate(44100.0f), // Set a default sample rate
//      bufferSize(10000),
//      bufferIndex(0),
//      centralFrequency(0),
//      bandwidth(0)
//{
//    buffer.resize(bufferSize);
//    frequencies.resize(numSinWaves);
//    amplitudes.resize(numSinWaves);
//    phases.resize(numSinWaves, 0.0f);          // Initialize phases
//    phaseIncrements.resize(numSinWaves, 0.0f); // Initialize phase increments
//    fillBuffer();
//}
//
//std::pair<float, float> SpatialNoiseGenerator::getNextSample()
//{
//    float sample = buffer[bufferIndex];
//    bufferIndex++;
//    if (bufferIndex >= bufferSize)
//        bufferIndex = 0;
//
//    if (bufferIndex == 0)
//    {
//        fillBuffer();
//    }
//
//    return std::make_pair(sample, sample);
//}
//
//void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
//{
//    sampleRate = newSampleRate;
//    
//    for (int i = 0; i < numSinWaves; ++i)
//    {
//        float minFreq = 20.0f;
//        float maxFreq = 20000.0f;
//        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq)); // Generate random frequency
//    }
//}
//
//void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
//{
//    this->amplCurve = amplCurve;
//}
//
//void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
//{
//    centralFrequency = centralFreq;
//    bandwidth = bw;
//    this->bwHeadFactor = bwHeadFactor;
//    this->bwTailFactor = bwTailFactor;
//}
//
//void SpatialNoiseGenerator::fillBuffer()
//{
//    for (int i = 0; i < numSinWaves; ++i)
//    {
////        float minFreq = 20.0f;
////        float maxFreq = 20000.0f;
////        frequencies[i] = minFreq * std::pow(10.0f, random.nextFloat() * std::log10(maxFreq / minFreq)); // Generate random frequency
//
//        if (bandwidth == 0)
//        {
//            amplitudes[i] = 0;
//            continue;
//        }
//
//        amplitudes[i] = juce::Decibels::decibelsToGain(amplCurve.valueAtFrequency(frequencies[i]) +
//                                                       -4.5f * std::log2(frequencies[i] / 1000.0f)); // Pink noise
//        float freq = frequencies[i];
//        float logDistance = std::abs(std::log2(freq / centralFrequency));
//        if (freq > centralFrequency) // Long head
//            logDistance *= bwHeadFactor;
//        else
//            logDistance *= bwTailFactor;
//
//        logDistance *= logDistance;
//        float logRatio = logDistance / bandwidth;
//
//        float bandpassGain = std::exp(-5.0f * logRatio);
//        amplitudes[i] *= bandpassGain;
//
//        // Compute phase increment for each sine wave
//        phaseIncrements[i] = frequencies[i] * (2.0f * juce::MathConstants<float>::pi) / sampleRate;
//    }
//
//    // Fill the buffer with noise samples
//    for (int i = 0; i < bufferSize; ++i)
//    {
//        float sample = 0.0f;
//
//        for (int j = 0; j < numSinWaves; ++j)
//        {
//            phases[j] += phaseIncrements[j];
//            if (phases[j] >= juce::MathConstants<float>::twoPi)
//                phases[j] -= juce::MathConstants<float>::twoPi;
//
//            sample += sineTable.get(phases[j]) * amplitudes[j];
//        }
//
//        buffer[i] = sample / numSinWaves;
//        buffer[i] *= 50;
//    }
//}
