/*
  ==============================================================================

    SpatialNoiseGenerator.cpp
    Created: 21 Nov 2024 11:03:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialNoiseGenerator.h"

SpatialNoiseGenerator::SpatialNoiseGenerator()
    : sampleRate(44100.0f),
      centralFrequency(0.0f),
      bandwidth(0.0f),
      bwHeadFactor(1.0f),
      bwTailFactor(1.0f),
      random()
{
    // Initialize phasors with random phases
    for (int i = 0; i < numSimdRegisters; ++i)
    {
        alignas(juce::dsp::SIMDRegister<float>) float realValues[simdSize];
        alignas(juce::dsp::SIMDRegister<float>) float imagValues[simdSize];

        for (int j = 0; j < simdSize; ++j)
        {
            int idx = i * simdSize + j;
            if (idx >= numSinWaves)
            {
                realValues[j] = 0.0f;
                imagValues[j] = 0.0f;
                continue;
            }

            float initialPhase = random.nextFloat() * (2.0f * juce::MathConstants<float>::pi);
            realValues[j] = std::cos(initialPhase);
            imagValues[j] = std::sin(initialPhase);
        }

        phasorReal[i] = juce::dsp::SIMDRegister<float>::fromRawArray(realValues);
        phasorImag[i] = juce::dsp::SIMDRegister<float>::fromRawArray(imagValues);

        // Initialize amplitudes to zero
        leftAmps[i] = juce::dsp::SIMDRegister<float>(0.0f);
        rightAmps[i] = juce::dsp::SIMDRegister<float>(0.0f);
    }
}

void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;

    // Initialize frequencies and phase increments
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logStep = (logMax - logMin) / (numSinWaves - 1);

    for (int i = 0; i < numSimdRegisters; ++i)
    {
        alignas(juce::dsp::SIMDRegister<float>) float phaseIncRealValues[simdSize];
        alignas(juce::dsp::SIMDRegister<float>) float phaseIncImagValues[simdSize];

        for (int j = 0; j < simdSize; ++j)
        {
            int idx = i * simdSize + j;
            if (idx >= numSinWaves)
            {
                phaseIncRealValues[j] = 1.0f; // No increment
                phaseIncImagValues[j] = 0.0f;
                continue;
            }

            // Calculate frequency
            frequencies[idx] = std::pow(10.0f, logMin + idx * logStep);

            // Calculate phase increment
            float phaseIncrement = frequencies[idx] * (2.0f * juce::MathConstants<float>::pi) / sampleRate;
            phaseIncRealValues[j] = std::cos(phaseIncrement);
            phaseIncImagValues[j] = std::sin(phaseIncrement);
        }

        phaseIncReal[i] = juce::dsp::SIMDRegister<float>::fromRawArray(phaseIncRealValues);
        phaseIncImag[i] = juce::dsp::SIMDRegister<float>::fromRawArray(phaseIncImagValues);
    }
}

std::pair<float, float> SpatialNoiseGenerator::getNextSample()
{
    juce::dsp::SIMDRegister<float> leftSampleVec(0.0f);
    juce::dsp::SIMDRegister<float> rightSampleVec(0.0f);

    for (int i = 0; i < numSimdRegisters; ++i)
    {
        // Update phasor: phasor *= phaseIncrement
        juce::dsp::SIMDRegister<float> tempReal = phasorReal[i] * phaseIncReal[i] - phasorImag[i] * phaseIncImag[i];
        juce::dsp::SIMDRegister<float> tempImag = phasorReal[i] * phaseIncImag[i] + phasorImag[i] * phaseIncReal[i];

        phasorReal[i] = tempReal;
        phasorImag[i] = tempImag;

        // Get sine value (imaginary part)
        juce::dsp::SIMDRegister<float> sinVal = phasorImag[i];

        // Multiply by amplitudes
        juce::dsp::SIMDRegister<float> leftVal = sinVal * leftAmps[i];
        juce::dsp::SIMDRegister<float> rightVal = sinVal * rightAmps[i];

        // Accumulate samples
        leftSampleVec += leftVal;
        rightSampleVec += rightVal;
    }

    // Sum all elements in leftSampleVec and rightSampleVec
    float leftSample = leftSampleVec.sum();
    float rightSample = rightSampleVec.sum();

    // Normalize
    leftSample /= static_cast<float>(numSinWaves);
    rightSample /= static_cast<float>(numSinWaves);

    leftSample *= 50.0f;
    rightSample *= 50.0f;

    return { leftSample * leftGain, rightSample * rightGain };
}

void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
{
    centralFrequency = centralFreq;
    bandwidth = bw;
    this->bwHeadFactor = bwHeadFactor;
    this->bwTailFactor = bwTailFactor;

    // Precompute amplitudes
    for (int i = 0; i < numSimdRegisters; ++i)
    {
        alignas(juce::dsp::SIMDRegister<float>) float leftAmpValues[simdSize];
        alignas(juce::dsp::SIMDRegister<float>) float rightAmpValues[simdSize];

        for (int j = 0; j < simdSize; ++j)
        {
            int idx = i * simdSize + j;
            if (idx >= numSinWaves)
            {
                leftAmpValues[j] = 0.0f;
                rightAmpValues[j] = 0.0f;
                continue;
            }

            if (bandwidth == 0.0f)
            {
                leftAmpValues[j] = 0.0f;
                rightAmpValues[j] = 0.0f;
                continue;
            }

            float freq = frequencies[idx];
            float amplVal = 0.0f;
            float panVal = 0.0f;
            
//            if (amplCurve.has_value())
//                amplCurve->valueAtFrequency(freq);
//            
//            if (panCurve.has_value())
//                panCurve->valueAtFrequency(freq);

            float cabinNoiseDropoff = -3.0f * std::log2(freq / 1000.0f); // pink noise based
            float leftAmpl = juce::Decibels::decibelsToGain(amplVal - 0.5f * panVal + cabinNoiseDropoff);
            float rightAmpl = juce::Decibels::decibelsToGain(amplVal + 0.5f * panVal + cabinNoiseDropoff);

            float logDistance = std::abs(std::log2(freq / centralFrequency));
            if (freq > centralFrequency) // Long head
                logDistance *= bwHeadFactor;
            else
                logDistance *= bwTailFactor;

            float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
            float slope = -24.0f / bandwidth;
            float bandpassDBChange = logDistance * slope;
            bandpassGain *= juce::Decibels::decibelsToGain (bandpassDBChange);
            
            leftAmpValues[j] = leftAmpl * bandpassGain;
            rightAmpValues[j] = rightAmpl * bandpassGain;
        }

        leftAmps[i] = juce::dsp::SIMDRegister<float>::fromRawArray(leftAmpValues);
        rightAmps[i] = juce::dsp::SIMDRegister<float>::fromRawArray(rightAmpValues);
    }
}

void SpatialNoiseGenerator::setPan (float pan)
{
    this->pan = pan;
    float angle = (pan + 1.0f) * M_PI / 4.0f;
    leftGain = std::cos (angle);
    rightGain = std::sin (angle);
}

void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
{
    this->amplCurve = amplCurve;
}

void SpatialNoiseGenerator::setPanCurve(Curve panCurve)
{
    this->panCurve = panCurve;
}

void SpatialNoiseGenerator::setPhaseCurve(Curve phaseCurve)
{
    this->phaseCurve = phaseCurve;
}
