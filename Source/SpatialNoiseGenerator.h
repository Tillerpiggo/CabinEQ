/////*
////  ==============================================================================
////
////    SpatialNoiseGenerator.h
////    Created: 12 Sep 2024 11:10:34pm
////    Author:  Tyler Gee
////
////  ==============================================================================
////*/
////
////#pragma once
////
////#include <JuceHeader.h>
////#include "Curve.h"
////
////class SineLookupTable {
////public:
////    SineLookupTable(int tableSize = 2048) // Increased table size for better accuracy
////        : tableSize(tableSize), lookupTable(tableSize)
////    {
////        const float twoPi = 2.0f * juce::MathConstants<float>::pi;
////        for (int i = 0; i < tableSize; ++i) {
////            lookupTable[i] = std::sin(i * twoPi / tableSize);
////        }
////    }
////
////    float get(float angle) const
////    {
////        // Normalize angle to [0, 2pi]
////        angle = std::fmod(angle, juce::MathConstants<float>::twoPi);
////        if (angle < 0) angle += juce::MathConstants<float>::twoPi;
////
////        float index = angle * tableSize / juce::MathConstants<float>::twoPi;
////        int lowerIndex = static_cast<int>(index) % tableSize;
////        int upperIndex = (lowerIndex + 1) % tableSize;
////
////        float fraction = index - lowerIndex;
////        return lookupTable[lowerIndex] * (1.0f - fraction) + lookupTable[upperIndex] * fraction;
////    }
////
////private:
////    int tableSize;
////    std::vector<float> lookupTable;
////};
////
////class SpatialNoiseGenerator {
////public:
////    SpatialNoiseGenerator();
////
////    std::pair<float, float> getNextSample();
////    void setSampleRate(float newSampleRate);
////    void setAmplCurve(Curve amplCurve);
////    void setPanCurve(Curve panCurve);
////    void setPhaseCurve(Curve phaseCurve);
////    void setBandpass(float centralFreq, float bandwidth, float bwHeadFactor, float bwTailFactor);
////
////private:
////    void fillBuffer();
////
////    float sampleRate;
////    int bufferSize;
////    int bufferIndex;
////    std::vector<float> buffer;
////    juce::Random random;
////    static const int numSinWaves = 100;
////    std::vector<float> frequencies;
////    std::vector<std::pair<float, float>> amplitudes;
////    std::vector<float> phases;          // Added phase storage
////    std::vector<float> phaseIncrements; // Added phase increment storage
////    int crossfadeLength = 500;
////
////    Curve amplCurve;
////    Curve panCurve;
////    Curve phaseCurve;
////    juce::IIRFilter bandpassFilter;
////    float centralFrequency;
////    float bandwidth;
////    float bwHeadFactor;
////    float bwTailFactor;
////    bool toggle;
////    SineLookupTable sineTable;
////};
//
//// SpatialNoiseGenerator.h
//
//#pragma once
//
//#include <JuceHeader.h>
//#include "Curve.h"
//#include <xsimd/xsimd.hpp>
//
//class SpatialNoiseGenerator {
//public:
//    SpatialNoiseGenerator();
//
//    std::pair<float, float> getNextSample();
//    void setSampleRate(float newSampleRate);
//    void setAmplCurve(Curve amplCurve);
//    void setPanCurve(Curve panCurve);
//    void setPhaseCurve(Curve phaseCurve);
//    void setBandpass(float centralFreq, float bandwidth, float bwHeadFactor, float bwTailFactor);
//
//private:
//    float sampleRate;
//    static const int numSinWaves = 128; // Adjust as needed
//
//    // Use arrays of structures for SIMD compatibility
//    struct SineWave {
//        xsimd::batch<float> realPart;     // Real part of phasor (SIMD)
//        xsimd::batch<float> imagPart;     // Imaginary part of phasor (SIMD)
//        xsimd::batch<float> phaseIncReal; // Real part of phase increment (SIMD)
//        xsimd::batch<float> phaseIncImag; // Imaginary part of phase increment (SIMD)
//        xsimd::batch<float> leftAmp;      // Left amplitude (SIMD)
//        xsimd::batch<float> rightAmp;     // Right amplitude (SIMD)
//    };
//
//    std::vector<SineWave> sineWaves; // Vector of sine waves
//    Curve amplCurve;
//    Curve panCurve;
//    Curve phaseCurve;
//    float centralFrequency;
//    float bandwidth;
//    float bwHeadFactor;
//    float bwTailFactor;
//    juce::Random random;
//};

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

private:
    float sampleRate = 44100.0f;
    static constexpr int numSinWaves = 256; // Adjust as needed

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

    Curve amplCurve;
    Curve panCurve;
    Curve phaseCurve;
    float centralFrequency = 0.0f;
    float bandwidth = 0.0f;
    float bwHeadFactor = 1.0f;
    float bwTailFactor = 1.0f;
    juce::Random random;
};
