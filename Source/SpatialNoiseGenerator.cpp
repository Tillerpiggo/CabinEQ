/////*
////  ==============================================================================
////
////    SpatialNoiseGenerator.cpp
////    Created: 12 Sep 2024 11:10:34pm
////    Author:  Tyler Gee
////
////  ==============================================================================
////*/
////
////#include "SpatialNoiseGenerator.h"
////
////SpatialNoiseGenerator::SpatialNoiseGenerator()
////    : sampleRate(44100.0f), // Set a default sample rate
////      centralFrequency(0),
////      bandwidth(0)
////{
////    frequencies.resize(numSinWaves);
////    amplitudes.resize(numSinWaves);
////    phases.resize(numSinWaves);
////    phaseIncrements.resize(numSinWaves, 0.0f);
////
////    juce::Random random; // JUCE random number generator
////
////    for (int i = 0; i < numSinWaves; ++i)
////    {
////        // Assign random phase between 0 and 2π
////        phases[i] = random.nextFloat() * (2.0f * juce::MathConstants<float>::pi);
////    }
////}
////
////std::pair<float, float> SpatialNoiseGenerator::getNextSample()
////{
////    float leftSample = 0.0f;
////    float rightSample = 0.0f;
////
////    for (int j = 0; j < numSinWaves; ++j)
////    {
////        phases[j] += phaseIncrements[j];
////        if (phases[j] >= juce::MathConstants<float>::twoPi)
////            phases[j] -= juce::MathConstants<float>::twoPi;
////        
////        float sinVal = sineTable.get(phases[j]);
////        std::pair<float, float> ampl = amplitudes[j];
////        leftSample += sinVal * ampl.first;
////        rightSample += sinVal * ampl.second;
////    }
////
////    leftSample /= numSinWaves;
////    rightSample /= numSinWaves;
////    leftSample *= 50;
////    rightSample *= 50;
////
////    return { leftSample, rightSample };
////}
////
////void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
////{
////    sampleRate = newSampleRate;
////    
////    juce::Random random;
////    
////    for (int i = 0; i < numSinWaves; ++i)
////    {
////        float minFreq = 20.0f;
////        float maxFreq = 20000.0f;
////        float logMin = std::log10(minFreq);
////        float logMax = std::log10(maxFreq);
////        
////        minFreq += random.nextFloat() * 5.0f;
////        
////        // Calculate the logarithmic step
////        float logStep = (logMax - logMin) / (numSinWaves - 1);
////        
////        // Calculate the frequency for each wave
////        frequencies[i] = std::pow(10.0f, logMin + i * logStep);
////        phaseIncrements[i] = frequencies[i] * (2.0f * juce::MathConstants<float>::pi) / sampleRate;
////    }
////}
////
////
////void SpatialNoiseGenerator::setAmplCurve(Curve amplCurve)
////{
////    this->amplCurve = amplCurve;
////}
////
////void SpatialNoiseGenerator::setPanCurve (Curve panCurve)
////{
////    this->panCurve = panCurve;
////}
////
////void SpatialNoiseGenerator::setPhaseCurve (Curve phaseCurve)
////{
////    this->phaseCurve = phaseCurve;
////}
////
////void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
////{
////    centralFrequency = centralFreq;
////    bandwidth = bw;
////    this->bwHeadFactor = bwHeadFactor;
////    this->bwTailFactor = bwTailFactor;
////
////    for (int i = 0; i < numSinWaves; ++i)
////    {
////        if (bandwidth == 0)
////        {
////            amplitudes[i] = { 0, 0 };
////            continue;
////        }
////        
////        float amplVal = amplCurve.valueAtFrequency (frequencies[i]);
//////        float panVal = panCurve.valueAtFrequency (frequencies[i]);
//////        float amplVal = 0;
////        float panVal = 0;
////        
////        float cabinNoiseDropoff = -4.5 * std::log2 (frequencies[i] / 1000.0f);
////        float leftAmpl = juce::Decibels::decibelsToGain (amplVal - 0.5 * panVal + cabinNoiseDropoff);
////        float rightAmpl = juce::Decibels::decibelsToGain (amplVal + 0.5 * panVal + cabinNoiseDropoff);
////        amplitudes[i] = { leftAmpl, rightAmpl };
////        float freq = frequencies[i];
////        float logDistance = std::abs(std::log2(freq / centralFrequency));
////        if (freq > centralFrequency) // Long head
////            logDistance *= bwHeadFactor;
////        else
////            logDistance *= bwTailFactor;
////        
////        float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
////        float slope = -24.0f / bandwidth;
////        float bandpassDBChange = logDistance * slope;
//////        bandpassGain *= juce::Decibels::decibelsToGain (bandpassDBChange);
////        amplitudes[i].first *= bandpassGain;
////        amplitudes[i].second *= bandpassGain;
////    }
////}
////
//
//// SpatialNoiseGenerator.cpp
//
//#include "SpatialNoiseGenerator.h"
//
//SpatialNoiseGenerator::SpatialNoiseGenerator()
//    : sampleRate(44100.0f), // Set a default sample rate
//      centralFrequency(0),
//      bandwidth(0)
//{
//    constexpr size_t simd_size = xsimd::simd_type<float>::size; // SIMD size (4, 8, or 16 depending on architecture)
//    const size_t numSimdVectors = (numSinWaves + simd_size - 1) / simd_size; // Ceiling division
//    sineWaves.resize(numSimdVectors);
//
//    random = juce::Random();
//
//    // Initialize phasors and phase increments
//    for (size_t i = 0; i < sineWaves.size(); ++i)
//    {
//        alignas(xsimd::default_arch::alignment()) float phasesReal[simd_size];
//        alignas(xsimd::default_arch::alignment()) float phasesImag[simd_size];
//        alignas(xsimd::default_arch::alignment()) float phaseIncReal[simd_size];
//        alignas(xsimd::default_arch::alignment()) float phaseIncImag[simd_size];
//
//        for (size_t j = 0; j < simd_size; ++j)
//        {
//            size_t idx = i * simd_size + j;
//            if (idx >= numSinWaves) break;
//
//            float initialPhase = random.nextFloat() * (2.0f * juce::MathConstants<float>::pi);
//            phasesReal[j] = std::cos(initialPhase);
//            phasesImag[j] = std::sin(initialPhase);
//
//            // Placeholder frequencies; actual frequencies will be set in setSampleRate()
//            phaseIncReal[j] = 1.0f;
//            phaseIncImag[j] = 0.0f;
//        }
//
//        sineWaves[i].realPart = xsimd::load_aligned(phasesReal);
//        sineWaves[i].imagPart = xsimd::load_aligned(phasesImag);
//        sineWaves[i].phaseIncReal = xsimd::load_aligned(phaseIncReal);
//        sineWaves[i].phaseIncImag = xsimd::load_aligned(phaseIncImag);
//        sineWaves[i].leftAmp = xsimd::batch<float>(0.0f);
//        sineWaves[i].rightAmp = xsimd::batch<float>(0.0f);
//    }
//}
//
//void SpatialNoiseGenerator::setSampleRate(float newSampleRate)
//{
//    sampleRate = newSampleRate;
//
//    constexpr size_t simd_size = xsimd::simd_type<float>::size;
//
//    // Recalculate phase increments based on the new sample rate
//    for (size_t i = 0; i < sineWaves.size(); ++i)
//    {
//        alignas(xsimd::default_arch::alignment()) float phaseIncReal[simd_size];
//        alignas(xsimd::default_arch::alignment()) float phaseIncImag[simd_size];
//        alignas(xsimd::default_arch::alignment()) float frequencies[simd_size];
//
//        for (size_t j = 0; j < simd_size; ++j)
//        {
//            size_t idx = i * simd_size + j;
//            if (idx >= numSinWaves) break;
//
//            float minFreq = 20.0f;
//            float maxFreq = 20000.0f;
//            float logMin = std::log10(minFreq);
//            float logMax = std::log10(maxFreq);
//            float logStep = (logMax - logMin) / (numSinWaves - 1);
//
//            frequencies[j] = std::pow(10.0f, logMin + idx * logStep);
//
//            float phaseIncrement = frequencies[j] * (2.0f * juce::MathConstants<float>::pi) / sampleRate;
//            phaseIncReal[j] = std::cos(phaseIncrement);
//            phaseIncImag[j] = std::sin(phaseIncrement);
//        }
//
//        sineWaves[i].phaseIncReal = xsimd::load_aligned(phaseIncReal);
//        sineWaves[i].phaseIncImag = xsimd::load_aligned(phaseIncImag);
//    }
//}
//
//std::pair<float, float> SpatialNoiseGenerator::getNextSample()
//{
//    xsimd::batch<float> leftSampleVec(0.0f);
//    xsimd::batch<float> rightSampleVec(0.0f);
//
//    for (size_t i = 0; i < sineWaves.size(); ++i)
//    {
//        // Update phasor: phasor *= phaseIncrement
//        xsimd::batch<float> tempReal = sineWaves[i].realPart * sineWaves[i].phaseIncReal - sineWaves[i].imagPart * sineWaves[i].phaseIncImag;
//        xsimd::batch<float> tempImag = sineWaves[i].realPart * sineWaves[i].phaseIncImag + sineWaves[i].imagPart * sineWaves[i].phaseIncReal;
//
//        sineWaves[i].realPart = tempReal;
//        sineWaves[i].imagPart = tempImag;
//
//        // Get sine value (imaginary part)
//        xsimd::batch<float> sinVal = sineWaves[i].imagPart;
//
//        // Multiply by amplitudes
//        xsimd::batch<float> leftVal = sinVal * sineWaves[i].leftAmp;
//        xsimd::batch<float> rightVal = sinVal * sineWaves[i].rightAmp;
//
//        // Accumulate samples
//        leftSampleVec += leftVal;
//        rightSampleVec += rightVal;
//    }
//
//    // Sum all elements in leftSampleVec and rightSampleVec
//    float leftSample = xsimd::hadd(leftSampleVec);
//    float rightSample = xsimd::hadd(rightSampleVec);
//
//    // Normalize
//    leftSample /= numSinWaves;
//    rightSample /= numSinWaves;
//
//    leftSample *= 50.0f;
//    rightSample *= 50.0f;
//
//    return { leftSample, rightSample };
//}
//
//void SpatialNoiseGenerator::setBandpass(float centralFreq, float bw, float bwHeadFactor, float bwTailFactor)
//{
//    centralFrequency = centralFreq;
//    bandwidth = bw;
//    this->bwHeadFactor = bwHeadFactor;
//    this->bwTailFactor = bwTailFactor;
//
//    constexpr size_t simd_size = xsimd::simd_type<float>::size;
//
//    for (size_t i = 0; i < sineWaves.size(); ++i)
//    {
//        alignas(xsimd::default_arch::alignment()) float frequencies[simd_size];
//        alignas(xsimd::default_arch::alignment()) float leftAmps[simd_size];
//        alignas(xsimd::default_arch::alignment()) float rightAmps[simd_size];
//
//        for (size_t j = 0; j < simd_size; ++j)
//        {
//            size_t idx = i * simd_size + j;
//            if (idx >= numSinWaves) break;
//
//            float minFreq = 20.0f;
//            float maxFreq = 20000.0f;
//            float logMin = std::log10(minFreq);
//            float logMax = std::log10(maxFreq);
//            float logStep = (logMax - logMin) / (numSinWaves - 1);
//
//            frequencies[j] = std::pow(10.0f, logMin + idx * logStep);
//
//            if (bandwidth == 0)
//            {
//                leftAmps[j] = 0.0f;
//                rightAmps[j] = 0.0f;
//                continue;
//            }
//
//            float amplVal = amplCurve.valueAtFrequency(frequencies[j]);
//            float panVal = panCurve.valueAtFrequency(frequencies[j]);
//
//            float cabinNoiseDropoff = -4.5f * std::log2(frequencies[j] / 1000.0f);
//            float leftAmpl = juce::Decibels::decibelsToGain(amplVal - 0.5f * panVal + cabinNoiseDropoff);
//            float rightAmpl = juce::Decibels::decibelsToGain(amplVal + 0.5f * panVal + cabinNoiseDropoff);
//
//            float freq = frequencies[j];
//            float logDistance = std::abs(std::log2(freq / centralFrequency));
//            if (freq > centralFrequency) // Long head
//                logDistance *= bwHeadFactor;
//            else
//                logDistance *= bwTailFactor;
//
//            float bandpassGain = (logDistance / bandwidth > 1.0f) ? 0.0f : 1.0f;
//
//            // Apply bandpass gain
//            leftAmps[j] = leftAmpl * bandpassGain;
//            rightAmps[j] = rightAmpl * bandpassGain;
//        }
//
//        sineWaves[i].leftAmp = xsimd::load_aligned(leftAmps);
//        sineWaves[i].rightAmp = xsimd::load_aligned(rightAmps);
//    }
//}

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
        float realValues[simdSize];
        float imagValues[simdSize];

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
        float phaseIncRealValues[simdSize];
        float phaseIncImagValues[simdSize];

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

    return { leftSample, rightSample };
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
        float leftAmpValues[simdSize];
        float rightAmpValues[simdSize];

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
            float amplVal = amplCurve.valueAtFrequency(freq);
            float panVal = panCurve.valueAtFrequency(freq);

            float cabinNoiseDropoff = -4.5f * std::log2(freq / 1000.0f);
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
