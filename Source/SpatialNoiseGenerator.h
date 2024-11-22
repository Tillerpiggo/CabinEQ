/*
  =============================================================================

    SpatialNoiseGenerator.
    Created: 21 Nov 2024 11:03:45p
    Author:  Tyler Ge

  =============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class SpatialNoiseGenerator {
public:
    SpatialNoiseGenerator();

    std::pair<float, float> getNextSample();
    void setSampleRate(float newSampleRate);
    void setAmplCurve(Curve amplCurve);
    void setPanCurve(Curve panCurve);
    void setPhaseCurve(Curve phaseCurve);
    void setBandpass(float centralFreq, float bandwidth, float bwHeadFactor, float bwTailFactor);
    void setPan (float pan);

private:
    float sampleRate = 44100.0f;
    float pan = 0.0f;
    float leftGain = 0.0f;
    float rightGain = 0.0f;
    static constexpr int numSinWaves = 20; // Adjust as needed

    // Number of SIMD registers based on SIMD size and the number of sine waves
    static constexpr int simdSize = juce::dsp::SIMDRegister<float>::size();
    static constexpr int numSimdRegisters = (numSinWaves + simdSize - 1) / simdSize;

    // Arrays to hold SIMD registers for phasors, phase increments, amplitudes
    juce::dsp::SIMDRegister<float> phasorReal[numSimdRegisters];
    juce::dsp::SIMDRegister<float> phasorImag[numSimdRegisters];
    juce::dsp::SIMDRegister<float> phaseIncReal[numSimdRegisters];
    juce::dsp::SIMDRegister<float> phaseIncImag[numSimdRegisters];
    juce::dsp::SIMDRegister<float> leftAmps[numSimdRegisters];
    juce::dsp::SIMDRegister<float> rightAmps[numSimdRegisters];
    float frequencies[numSinWaves]; // Store frequencies for calculations

   std::optional<Curve> amplCurve;
   std::optional<Curve> panCurve;
   std::optional<Curve> phaseCurve;
    float centralFrequency = 0.0f;
    float bandwidth = 0.0f;
    float bwHeadFactor = 1.0f;
    float bwTailFactor = 1.0f;
    juce::Random random;
};
