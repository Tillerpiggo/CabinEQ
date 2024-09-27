/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : filter (FFT_SIZE),
      arbitrarySequencer (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer2 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer3 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer4 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      arbitrarySequencer5 (std::make_unique<SineWaveGenerator> (SineWaveGenerator())),
      isTesting (false),
      isSweeping (false),
      isCalibrating (false),
      isProcessing (false),
      hasPreparedFilter (false)
{
    gainProcessor.setRampDurationSeconds (0.05);
    gainProcessor.setGainDecibels (0.0f);
    
    // Eight ascending notes, adjusting the harmonic
//    float freqFactorBelow = 2.0;
//    float bandwidth = 3.0;
//    std::pair<float, float> overtoneEnvelope { 1.5, 0.3 }; // long tail
//    std::pair<float, float> undertoneEnvelope { 0.3, 1.5 }; // long head
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> harmonicPattern {
//        NoiseNote (std::pow (freqFactorBelow, -4), bandwidth, durationInSamples, 0.0, overtoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, -3), bandwidth, durationInSamples, 0.0, overtoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, -2), bandwidth, durationInSamples, 0.0, overtoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, -1), bandwidth, durationInSamples, 0.0, overtoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, 1), bandwidth, durationInSamples, 0.0, undertoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, 2), bandwidth, durationInSamples, 0.0, undertoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, 3), bandwidth, durationInSamples, 0.0, undertoneEnvelope),
//        NoiseNote (std::pow (freqFactorBelow, 4), bandwidth, durationInSamples, 0.0, undertoneEnvelope),
//    };
//    spatialPatternGenerator.setPattern (harmonicPattern);
    
    // W played with upper harmonics
//    float freqFactor = 0.5;
//    float bandwidth = 3.0;
//    std::pair<float, float> harmonicEnvelope { 1.5, 0.3 }; // long tail for harmonics
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> wPattern {
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.75, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (wPattern);
    
    // M played with lower harmonics
//    float freqFactor = 2.0;
//    float bandwidth = 3.0;
//    std::pair<float, float> harmonicEnvelope { 0.3, 1.5 }; // long head for lower harmonics
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> mPattern {
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.75, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.25, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.75, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (mPattern);
    
    // X shape played with lower harmonics and upper harmonics
//    float freqFactor = 2.0;
//    float bandwidth = 3.0;
//    std::pair<float, float> harmonicEnvelope { 0.3, 1.5 }; // long head for lower harmonics
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> forwardSlashPattern {
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -0.6, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -0.4, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -0.2, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.2, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.4, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.6, harmonicEnvelope),
//    };
//    std::vector<NoiseNote> backslashPattern {
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -0.6, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, -0.4, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -0.2, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.2, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.4, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.6, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (forwardSlashPattern);
//    spatialPatternGenerator2.setPattern (backslashPattern);
    
    // Zigzag pattern
//    float freqFactor = 1.3;
//    float bandwidth = 2.5;
//    std::pair<float, float> harmonicEnvelope { 1.5, 0.3 }; // long tail
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> zigzagPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (zigzagPattern);
    
    // Zigzag pattern (lower harmonic)
//    float freqFactor = 1.1;
//    float bandwidth = 4.0;
//    std::pair<float, float> lowerHarmonicEnvelope { 0.3, 1.5 }; // long head
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> lowerHarmonicZigzagPattern {
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 4), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 6), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 8), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (lowerHarmonicZigzagPattern);
    
    // Harmonic Alternating
//    float freqFactor = 1.3;
//    float bandwidth = 2.0;
//    std::pair<float, float> harmonicEnvelope { 2.0, 0.2 }; // long tail
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> harmonicPattern {
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (harmonicPattern);
    
    // Lower zigzag 2 (with wider bandwidth and lower freqFactor and longer head)
//    float freqFactor = 1.05;
//    float bandwidth = 5.0;
//    std::pair<float, float> lowerHarmonicEnvelope { 0.3, 2.0 }; // long head
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> lowerHarmonicZigzagPattern {
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 4), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 6), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, -1.0, lowerHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 8), bandwidth, durationInSamples, 1.0, lowerHarmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (lowerHarmonicZigzagPattern);
    
    // Zigzag 2 (with wider bandwidth and lower freqFactor and much longer tail)
//    float freqFactor = 1.05;
//    float bandwidth = 4.0;
//    std::pair<float, float> harmonicEnvelope { 0.1, 8.0 }; // long head
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> harmonicZigzagPattern2 {
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 8), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (harmonicZigzagPattern2);
    
    // Zigzag 3 (with different values and correct)
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 }; // long head
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> harmonicZigzagPattern2 {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (harmonicZigzagPattern2);
    
    // Zigzag 4 (with notes in the center)
//    float freqFactor = 1.02;
//    float bandwidth = 4.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 }; // long head
//    int durationInSamples = 3000;
//    std::vector<NoiseNote> harmonicZigzagPattern2 {
//        NoiseNote (std::pow (freqFactor, -12), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -11), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -10), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -9), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (harmonicZigzagPattern2);
    
    // Rising columns
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicRisingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicRisingPattern);
    
    // Falling columns
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicFallingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicFallingPattern);
    
    // Rising columns 2
//    float freqFactor = 1.04;
//    float bandwidth = 8.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicRisingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicRisingPattern);
    
    // Rising columns 3
//    float freqFactor = 2.0;
//    float bandwidth = 3.0;
//    std::pair<float, float> harmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//    };
//    std::vector<NoiseNote> rightHarmonicRisingPattern {
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicRisingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicRisingPattern);
    
    // Converging columns
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> risingHarmonicEnvelope { 0.5, 2.0 };
//    std::pair<float, float> fallingHarmonicEnvelope { 2.0, 0.5 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> risingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope)
//    };
//    std::vector<NoiseNote> fallingPattern {
//        NoiseNote (std::pow (freqFactor, 8), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 6), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 4), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (risingPattern);
//    spatialPatternGenerator2.setPattern (fallingPattern);
    
    // Alternating converging columns
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> risingHarmonicEnvelope { 0.5, 2.0 };
//    std::pair<float, float> fallingHarmonicEnvelope { 2.0, 0.5 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> risingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (risingPattern);
    
    // Alternating converging columns (shorter bandwidth)
//    float freqFactor = 1.04;
//    float bandwidth = 2.0;
//    std::pair<float, float> risingHarmonicEnvelope { 0.5, 2.0 };
//    std::pair<float, float> fallingHarmonicEnvelope { 2.0, 0.5 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> risingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 7), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 5), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, fallingHarmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (risingPattern);
    
    // Alternating rising
//    float freqFactor = 1.05;
//    float bandwidth = 4.0;
//    std::pair<float, float> risingHarmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> risingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, risingHarmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (risingPattern);
    
    // Alertnate rising falling
//    float freqFactor = 1.04;
//    float bandwidth = 4.0;
//    std::pair<float, float> risingHarmonicEnvelope { 0.5, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> risingPattern {
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, risingHarmonicEnvelope),
//        
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, risingHarmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (risingPattern);
    
    // Falling columns 2
//    float freqFactor = 1.2;
//    float bandwidth = 0.5;
//    std::pair<float, float> harmonicEnvelope { 0.01, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicFallingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicFallingPattern);
//    
    // Falling columns 3, paused
//    float freqFactor = 1.2;
//    float offsetFactor = 2.0;
//    float bandwidth = 0.5;
//    std::pair<float, float> harmonicEnvelope { 0.01, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicFallingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicFallingPattern);
    
    // Falling columns 4, paused
//    float freqFactor = 1.2;
//    float offsetFactor = 2.0;
//    float bandwidth = 0.1;
//    std::pair<float, float> harmonicEnvelope { 0.01, 2.0 };
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> leftHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    std::vector<NoiseNote> rightHarmonicFallingPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -2) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -3) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -4) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -5) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -6) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -7) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -8), 0.0, durationInSamples * 3, -1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (leftHarmonicFallingPattern);
//    spatialPatternGenerator2.setPattern (rightHarmonicFallingPattern);
    
    // X, again
//    float bandwidth = 1.0;
//    float freqFactor = 1.7;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> xPattern2 {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern2);
    
    // X2, again
//    float bandwidth = 2.0;
//    float freqFactor = 1.7;
//    float offsetFactor = 0.7;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> xPattern2 {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern2);
    
    // X3, again
//    float bandwidth = 2.0;
//    float freqFactor = 1.3;
//    float offsetFactor = 1.3;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 2.0, 1.0 }; // long tail
//    std::vector<NoiseNote> xPattern3 {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern3);
    
    // X4
//    float bandwidth = 3.0;
//    float freqFactor = 1.3;
//    float offsetFactor = 1.0;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 }; // long tail
//    std::vector<NoiseNote> xPattern4 {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (0, 0, durationInSamples, 0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern4);
    
    // Reference tone
//    float bandwidth = 2.0;
//    float freqFactor = 1.3;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> xPattern4 {
//        NoiseNote (1000.0, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (1.0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern4);
    
    // Elevation calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> xPattern4 {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern4);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating || isTesting)
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5  * juce::Decibels::decibelsToGain (referenceVolume);
        }
    }
    else if (isTesting) // making testing sweep, for now
    {
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = sineSweepGenerator.getNextSample();
            float referenceGain = juce::Decibels::decibelsToGain (referenceVolume);
            referenceGain *= juce::Decibels::decibelsToGain (getCompensationDBAtFrequency (sineSweepGenerator.getCurrFreq()));
            
            leftChannel[sample] = value.first * 0.05 * 0.5 * referenceGain;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5 * referenceGain;
        }
    }
    else
    {
        juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
        auto ioContext = juce::dsp::ProcessContextReplacing<float> (ioBlock);
        ioContext.isBypassed = ! isProcessing; // convolution will handle the bypass appropriately in it's process method
        filter.process (ioContext);
        gainProcessor.process (ioContext);
    }
}

void PlaybackManager::updateFilterWithCurves (Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, int fftSize)
{
    filter.updateWithCurves (amplCurve, panCurve, phaseCurve, fftSize);
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    spatialPatternGenerator.setSampleRate (spec.sampleRate);
    spatialPatternGenerator2.setSampleRate (spec.sampleRate);
    spatialPatternGenerator3.setSampleRate (spec.sampleRate);
    spatialPatternGenerator4.setSampleRate (spec.sampleRate);
//    spatialPatternGenerator.prepare (spec);
    arbitrarySequencer.setSampleRate (spec.sampleRate);
    arbitrarySequencer2.setSampleRate (spec.sampleRate);
    arbitrarySequencer3.setSampleRate (spec.sampleRate);
    arbitrarySequencer4.setSampleRate (spec.sampleRate);
    arbitrarySequencer5.setSampleRate (spec.sampleRate);
    sineWaveGenerator1.setSampleRate (spec.sampleRate);
    sineWaveGenerator2.setSampleRate (spec.sampleRate);
    hasPreparedFilter = true;
}

float PlaybackManager::getCurrPlayingFreq() const
{
    return arbitrarySequencer.currentlyPlayingFrequency();
}

float PlaybackManager::getCurrTestingFreq() const
{
    return testingFreq;
}

float PlaybackManager::getCurrSineSweepFreq() const
{
    return sineSweepGenerator.getCurrFreq();
}

void PlaybackManager::setIsTesting (bool isTesting)
{
    this->isTesting = isTesting;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setIsSweeping (bool isSweeping)
{
    this->isSweeping = isSweeping;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
//    gainProcessor.setGainDecibels (isCalibrating ? wetVolume : dryVolume);
}

void PlaybackManager::setIsProcessing (bool isProcessing)
{
    this->isProcessing = isProcessing;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setDryWetVolumeBalance (float balance)
{
//    dryGainProcessor.setGainDecibels (-balance);
//    wetGainProcessor.setGainDecibels (+balance);
}

void PlaybackManager::setWetVolume (float wetVolume)
{
    this->wetVolume = wetVolume;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setDryVolume (float dryVolume)
{
    this->dryVolume = dryVolume;
    gainProcessor.setGainDecibels (isProcessing ? wetVolume : dryVolume);
}

void PlaybackManager::setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.setCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl)
{
//    sineSweepGenerator.updateCenterFrequency (centerFreq, ampl);
}

void PlaybackManager::startAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator.setCenterFrequency (freq);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setCenterFrequency (freq);
    spatialPatternGenerator3.setAmplCurve (amplCurve);
    spatialPatternGenerator3.setCenterFrequency (freq);
    spatialPatternGenerator4.setAmplCurve (amplCurve);
    spatialPatternGenerator4.setCenterFrequency (freq);
    
    // Create a frequency specific pattern to play a rising/falling sequence towards it
//    float centerFreq = 1000.0f;
//    float freqRatio = freq / centerFreq;
//    float freqFactor = std::pow (freqRatio, 1.0f / 8.0f);
//    float bandwidth = 3.0;
//    std::pair<float, float> lowerHarmonicEnvelope { 1.0, 1.0 }; // long head
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> expandingPattern {
//        NoiseNote (centerFreq * std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 3), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 4), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 5), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 6), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 7), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false),
//        NoiseNote (centerFreq * std::pow (freqFactor, 8), bandwidth, durationInSamples, 0.0, lowerHarmonicEnvelope, false)
//    };
//    spatialPatternGenerator.setPattern (expandingPattern);
    
    // Dynamic elevation calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    float offsetFactor;
//    if (freq <= 1000.0f)
//        offsetFactor = std::abs (std::log2 (freq / 1000.0f));
//    else
//        offsetFactor = 1.0f / std::abs (std::log2 (freq / 1000.0f));
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> xPattern4 {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (xPattern4);
    
    // Dynamic X calibration
//    float bandwidth = 1.0;
//    float freqFactor = 1.5;
//    float offsetFactor;
//    if (freq <= 1000.0f)
//        offsetFactor = std::abs (std::log2 (freq / 1000.0f));
//    else
//        offsetFactor = 1.0f / std::abs (std::log2 (freq / 1000.0f));
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicXPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (dynamicXPattern);
    
    // Dynamic X2 Calibration
//    float bandwidth = 0.5;
//    float freqFactor = 1.15;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX2Pattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (dynamicX2Pattern);
    
//    // with context
//    float contextFreqFactor = 2;
//    std::vector<NoiseNote> contextNoisePattern {
//        NoiseNote (std::pow (contextFreqFactor, -1), bandwidth, durationInSamples * 3, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples * 3, 0.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator2.setPattern (contextNoisePattern);
    
    // Interval calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.15;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> intervalPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (intervalPattern);
    
    // Interval calibration (wider)
//    float bandwidth = 4.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> intervalPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (intervalPattern);
    
//    // Dynamic interval calibration
//    std::cout << "freq: " << freq << std::endl;
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        // Grab the frequency in between the nodes
//        auto middleFreq = std::sqrt (lowerFreq * higherFreq);
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 4.0 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 4.0f);
//        int durationInSamples = 15000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope)
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Dynamic interval calibration 2
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        // Grab the frequency in between the nodes
//        auto middleFreq = std::sqrt (lowerFreq * higherFreq);
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 2.0 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 3.0f);
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope)
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Triangle calibration 1
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 2.0 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 3.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, -0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -4), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Triangle calibration 2
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 2.0 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 3.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Triangle calibration 3
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 1.0 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 2.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, -0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -3), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.33, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.66, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
//
    
    // Row-wise calibration
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 0.5 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 2.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Reference row calibration
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 1.0;//0.5 * freqRatio;
//        float freqFactor = std::pow (freqRatio, 1.0f / 2.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (1000, bandwidth, durationInSamples, -1.0, harmonicEnvelope, false),
//            NoiseNote (1000, bandwidth, durationInSamples, -0.5, harmonicEnvelope, false),
//            NoiseNote (1000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//            NoiseNote (1000, bandwidth, durationInSamples, 0.5, harmonicEnvelope, false),
//            NoiseNote (1000, bandwidth, durationInSamples, 1.0, harmonicEnvelope, false),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.5, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Relative Row Calibration
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        auto middleFreq = std::sqrt (higherFreq * lowerFreq);
//
//        auto freqRatio = higherFreq / lowerFreq;
//
//        float bandwidth = 1.0;
//        float freqFactor = std::pow (freqRatio, 1.0f / 2.0f);
//        int durationInSamples = 5000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> dynamicIntervalPattern {
//            NoiseNote (lowerFreq, bandwidth, durationInSamples, -1.0, harmonicEnvelope, false),
//            NoiseNote (lowerFreq, bandwidth, durationInSamples, -0.5, harmonicEnvelope, false),
//            NoiseNote (lowerFreq, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//            NoiseNote (lowerFreq, bandwidth, durationInSamples, 0.5, harmonicEnvelope, false),
//            NoiseNote (lowerFreq, bandwidth, durationInSamples, 1.0, harmonicEnvelope, false),
//            NoiseNote (middleFreq, bandwidth, durationInSamples, -1.0, harmonicEnvelope, false),
//            NoiseNote (middleFreq, bandwidth, durationInSamples, -0.5, harmonicEnvelope, false),
//            NoiseNote (middleFreq, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//            NoiseNote (middleFreq, bandwidth, durationInSamples, 0.5, harmonicEnvelope, false),
//            NoiseNote (middleFreq, bandwidth, durationInSamples, 1.0, harmonicEnvelope, false),
//            NoiseNote (higherFreq, bandwidth, durationInSamples, -1.0, harmonicEnvelope, false),
//            NoiseNote (higherFreq, bandwidth, durationInSamples, -0.5, harmonicEnvelope, false),
//            NoiseNote (higherFreq, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//            NoiseNote (higherFreq, bandwidth, durationInSamples, 0.5, harmonicEnvelope, false),
//            NoiseNote (higherFreq, bandwidth, durationInSamples, 1.0, harmonicEnvelope, false),
//        };
//        spatialPatternGenerator.setPattern (dynamicIntervalPattern);
//    }
//    else
//    {
//        float bandwidth = 4.0;
//        float freqFactor = 1.5;
//        int durationInSamples = 20000;
//        std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//        std::vector<NoiseNote> intervalPattern {
//            NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//            NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        };
//        spatialPatternGenerator.setPattern (intervalPattern);
//    }
    
    // Rectangular Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX2Pattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (dynamicX2Pattern);
    
    // Absolute Rising Calibration
//    float bandwidth = 2.0;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> absoluteRisingPattern {
//        NoiseNote (20, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (40, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (80, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (160, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (320, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (640, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (1280, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (2560, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (5120, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (10240, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false)
//    };
//    spatialPatternGenerator.setPattern (absoluteRisingPattern);
    
    // Dynamic X3 calibration
//    float bandwidth = 0.6;
//    float freqFactor = 1.7;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX3Pattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (dynamicX3Pattern);
    
    // Dynamic X4 calibration
//    float bandwidth = 0.6;
//    float freqFactor = 1.7;
//    float offsetFactor = 0.75;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX3Pattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (dynamicX3Pattern);
    
    // Dynamic X5 calibration
//    float bandwidth = 0.6;
//    float freqFactor = 1.7;
//    float offsetFactor = 0.5;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX3Pattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (dynamicX3Pattern);
    
    // Rectangular Calibration 3
//    float bandwidth = 2.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> dynamicX2Pattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, -1.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (dynamicX2Pattern);
    
    // Motion X Calibration
//    float bandwidth = 8.0;
//    float freqFactor = 1.1;
//    int durationInSamples = 4000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionXPattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionXPattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//    spatialPatternGenerator.setPattern (motionXPattern);
    
    // Motion X2 Calibration
//    float bandwidth = 4.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 4000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX2Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, -pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX2Pattern);
    
    // Motion X3 Calibration
//    float bandwidth = 5.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX2Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, -pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX2Pattern);
    
    // Motion X4 Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX2Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, -pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX2Pattern);
    
    // Rising Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX2Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX2Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, -pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX2Pattern);
    
    // Rising 2 Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    float panLeft = -1.0;
//    float panRight = 1.0;
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5,
//                                2.0, 1.5, 1.0, 0.5, 0.0, -0.5, -1.0, -1.5 };
//    std::vector<NoiseNote> risingPattern;
//    std::vector<NoiseNote> risingPattern2;
//
//    for (int i = 0; i < freqs.size(); ++i)
//    {
//        risingPattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, panLeft, harmonicEnvelope);
//    }
//
//    for (int i = 0; i < freqs.size(); ++i)
//    {
//        risingPattern2.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, panRight, harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (risingPattern);
//    spatialPatternGenerator2.setPattern (risingPattern2);
    
    // Circle Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 2.0;
//    float offsetFactor = 1.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8,
//                              1.0, 0.8, 0.6, 0.4, 0.2, 0.0, -0.2, -0.4, -0.6, -0.8 };
//    std::vector<NoiseNote> circlePattern;
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        circlePattern.emplace_back (std::pow (freqFactor, pans[i] + offsetFactor), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//    spatialPatternGenerator.setPattern (circlePattern);
    
    // Rising 3 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    float panLeft = -1.0;
//    float panRight = 1.0;
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5 };
//    std::vector<NoiseNote> risingPattern;
//    std::vector<NoiseNote> risingPattern2;
//
//    for (int i = 0; i < freqs.size(); ++i)
//    {
//        risingPattern.emplace_back (std::pow (freqFactor, freqs[i] - 1.5), bandwidth, durationInSamples, panLeft, harmonicEnvelope);
//    }
//
//    for (int i = 0; i < freqs.size(); ++i)
//    {
//        risingPattern2.emplace_back (std::pow (freqFactor, freqs[i] - 1.5), bandwidth, durationInSamples, panRight, harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (risingPattern);
//    spatialPatternGenerator2.setPattern (risingPattern2);
    
    // Motion X5 Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX5Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, -pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX5Pattern);
    
    // Parallel Motion Calibration
//    float bandwidth = 0.5;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
//    std::vector<float> freqs { -2.0, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<NoiseNote> motionX5Pattern;
//
//    for (int j = -1; j < 2; ++j)
//    {
//        for (int i = 0; i < pans.size(); ++i)
//        {
//            motionX5Pattern.emplace_back (std::pow (freqFactor, freqs[i] + j), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//        }
//    }
//
//    spatialPatternGenerator.setPattern (motionX5Pattern);
    
    // Spinny Noise
//    float bandwidth = 5.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0,
//                               0.8, 0.6, 0.4, 0.2, 0.0, -0.2, -0.4, -0.6, -0.8, -1.0 };
//    std::vector<NoiseNote> motionX5Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (500.0, bandwidth, durationInSamples, pans[i], harmonicEnvelope, false);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (1000.0, bandwidth, durationInSamples, pans[i], harmonicEnvelope, false);
//    }
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (2000.0, bandwidth, durationInSamples, pans[i], harmonicEnvelope, false);
//    }
//
//    spatialPatternGenerator.setPattern (motionX5Pattern);
    
    // Panning Back and Forth
//    float bandwidth = 2.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 2000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, 1.0 };
//    std::vector<float> freqs { 0.0, 0.0 };
//    std::vector<NoiseNote> motionX5Pattern;
//
//    for (int j = -1; j < 2; ++j)
//    {
//        for (int i = 0; i < pans.size(); ++i)
//        {
//            motionX5Pattern.emplace_back (std::pow (freqFactor, freqs[i]), bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//        }
//    }
//
//    spatialPatternGenerator.setPattern (motionX5Pattern);
    
    // Panning Back and Forth 2
//    float bandwidth = 2.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<float> freqs { 0.0, 0.0, 0.0, 0.0, 0.0 };
//    std::vector<NoiseNote> motionX5Pattern;
//
//    for (int i = 0; i < pans.size(); ++i)
//    {
//        motionX5Pattern.emplace_back (1.0, bandwidth, durationInSamples, pans[i], harmonicEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (motionX5Pattern);
    
    // Relative Elevation Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth * 5.0, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 2
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth * 5.0, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 2), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 3
//    float bandwidth = 1.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (200, bandwidth * 5.0, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (3000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (5000, bandwidth * 5.0, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (6000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 4
//    float bandwidth = 3.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (500, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (1000, bandwidth * 2.0, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (2000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (4000, bandwidth * 2.0, durationInSamples, 0.0, harmonicEnvelope, false),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 5
//    float bandwidth = 10.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (500, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (1000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (2000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//        NoiseNote (4000, bandwidth, durationInSamples, 0.0, harmonicEnvelope, false),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 6
//    float bandwidth = 10.0;
//    float freqFactor = 3.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Relative Elevation Calibration 7
//    float bandwidth = 3.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 6000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -0.5), bandwidth * 0.3, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth * 0.1, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0.5), bandwidth * 0.1, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (std::pow (freqFactor, -0.5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (std::pow (freqFactor, 0.5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);

//    std::vector<NoiseNote> relativeElevationPattern2 {
//        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
    //    spatialPatternGenerator2.setPattern (relativeElevationPattern2); // for comparison with original
    
    // Relative Elevation Calibration 8
//    float bandwidth = 3.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth * 3, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth * 0.2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth * 3, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0), bandwidth * 0.2, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Noise on Noise
//    float bandwidth = 1.0;
//    int durationInSamples = 10000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> noiseOnNoisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 7, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (noiseOnNoisePattern);
//    spatialPatternGenerator2.setPattern (noisePattern);
    
    // Noise on noise 2
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 20000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> noiseOnNoisePattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth * 4.0, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth * 4.0, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow (freqFactor, -1), bandwidth * 1.0, durationInSamples / 3, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1), bandwidth * 1.0, durationInSamples / 3, 0.0, harmonicEnvelope)
//    };
//    spatialPatternGenerator.setPattern (noiseOnNoisePattern);
//    spatialPatternGenerator2.setPattern (noisePattern);
    
    // Multibandwidth 2
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 1000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.2, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.3, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.4, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.5, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.6, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.7, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.8, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.9, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.9 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.8 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.7 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.6 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.5 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.4 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.3 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.2 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.1 })
//
////        NoiseNote (std::pow (freqFactor, -0.5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (std::pow (freqFactor, 0.5), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
////        NoiseNote (0, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth 3
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (5, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth 4
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (0.33, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.1 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
//        NoiseNote (3, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth 5 (from noise)
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.001, 0.001 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
////        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
////        NoiseNote (3, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth 6
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.001, 0.001 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth panning
//    float bandwidth = 1.0;
//    int durationInSamples = 3000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    for (int i = 1; i < 8; ++i)
//    {
//        float envelopeFactor = 1.0f / static_cast<float>(i);
//        std::pair<float, float> envelope = { envelopeFactor, envelopeFactor };
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, -1.0, envelope);
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, 1.0, envelope);
//    }
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
//    {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.001, 0.001 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
    
    // Multi-bandwidth centered flipping
//    float bandwidth = 1.0;
//    int durationInSamples = 3000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    for (int i = 1; i < 8; ++i)
//    {
//        float envelopeFactor = 1.0f / static_cast<float>(i);
//        std::pair<float, float> envelope = { envelopeFactor, 1.0 };
//        std::pair<float, float> envelope2 = { 1.0, envelopeFactor };
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, 0.0, envelope);
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, 0.0, envelope2);
//    }
//
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Spatial Flatness with Pink Noise
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 5000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.001, 0.001 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Feels Good with Cabin Noise (Panned)
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 2000;
//    std::pair<float, float> narrowbandEnvelope { bandwidth, bandwidth };
//    std::pair<float, float> allpassEnvelope { 0.001, 0.001 };
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    for (int i = -5; i <= 5; ++i)
//    {
//        float pan = static_cast<float> (i) / 5.0f;
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, pan, narrowbandEnvelope);
//        relativeElevationPattern.emplace_back (1, bandwidth, durationInSamples, pan, allpassEnvelope);
//    }
//
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi bandwidth nonsense
//    float bandwidth = 1.0;
//    float freqFactor = 1.3;
//    int durationInSamples = 2000;
//    std::pair<float, float> narrowbandEnvelope { bandwidth, bandwidth };
//    std::pair<float, float> allpassEnvelope { 0.001, 0.001 };
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    for (int i = 1; i <= 5; ++i)
//    {
//        relativeElevationPattern.emplace_back (1, bandwidth * i, durationInSamples, 0, narrowbandEnvelope);
//    }
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
//    // Multi-bandwidth L/R Above
//    float bandwidth = 2.0;
//    float freqFactor = 1.3;
//    float offsetFactor = 1.3;
//    int durationInSamples = 6000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -0.5) * offsetFactor, bandwidth * 0.3, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth * 0.1, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0.5) * offsetFactor, bandwidth * 0.1, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Multi-bandwidth L/R
    /*
    float bandwidth = 3.0;
    float freqFactor = 1.3;
    int durationInSamples = 6000;
    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
    std::vector<NoiseNote> relativeElevationPattern {
        NoiseNote (std::pow (freqFactor, -1), bandwidth * 2, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, -1), bandwidth * 2, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, -0.5), bandwidth * 0.3, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, -0.5), bandwidth * 0.3, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0), bandwidth * 0.1, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0), bandwidth * 0.1, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0), bandwidth, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0.5), bandwidth * 0.1, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 0.5), bandwidth * 0.1, durationInSamples, 1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, -1.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth * 2, durationInSamples, 1.0, harmonicEnvelope),
    };
    spatialPatternGenerator.setPattern (relativeElevationPattern);
     */
    
    // Multi-bandwidth Mono 2
//    float bandwidth = 2.0;
//    float freqFactor = 1.3;
//    float offsetFactor = 1.3;
//    int durationInSamples = 6000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (std::pow (freqFactor, -1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, -0.5) * offsetFactor, bandwidth * 0.5, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth * 0.1, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0.5) * offsetFactor, bandwidth * 0.5, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 1) * offsetFactor, bandwidth * 2, durationInSamples, 0.0, harmonicEnvelope),
//        NoiseNote (std::pow (freqFactor, 0) * offsetFactor, bandwidth * 3, durationInSamples, 0.0, harmonicEnvelope),
//    };
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
//    // Multi-bandwidth Mono 3
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 3000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> bandwidths { 0.25, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> freqFactors { -1, -0.5, 0, 0.5, 1 };
//    std::vector<NoiseNote> relativeElevationPattern;
//    
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currBandwidth : bandwidths)
//        {
//            
//            relativeElevationPattern.emplace_back (std::pow (freqFactor, currFreqFactor) * offsetFactor, currBandwidth, durationInSamples, 0.0, harmonicEnvelope);
//        }
//    }
//    
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Random-bandwidth // this is the one I was last using
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 6000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    int numNotes = 20; // Number of noise notes to generate
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    // Set up random number generators
//    std::random_device rd;
//    std::mt19937 gen(rd());
//
//    // Define ranges for random bandwidths and frequency offsets
//    std::uniform_real_distribution<> bandwidthDist(0.25, 2.0);   // Bandwidth between 0.25 and 2.0
//    std::uniform_real_distribution<> freqFactorDist(-1.0, 1.0);  // Frequency factor between -1.0 and 1.0
//
//    // Generate noise notes with random parameters
//    for (int i = 0; i < numNotes; ++i)
//    {
//        // Generate random frequency factor and bandwidth
//        float currFreqFactor = freqFactorDist(gen);
//        float currBandwidth = bandwidthDist(gen);
//
//        // Calculate the frequency using the random frequency factor
//        float frequency = std::pow(freqFactor, currFreqFactor) * offsetFactor;
//
//        // Create a NoiseNote with random parameters
//        relativeElevationPattern.emplace_back(frequency, currBandwidth, durationInSamples, 0.0f, harmonicEnvelope);
//    }
//
//    // Set the pattern with the generated noise notes
//    spatialPatternGenerator.setPattern(relativeElevationPattern);
//    
    // Random-bandwidth 2
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 3000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    int numNotes = 20; // Number of noise notes to generate
//    std::vector<NoiseNote> relativeElevationPattern;
//
//    // Set up random number generators
//    std::random_device rd;
//    std::mt19937 gen(rd());
//
//    // Define ranges for random bandwidths and frequency offsets
//    std::uniform_real_distribution<> bandwidthDist(0.1, 4.0);   // Bandwidth between 0.25 and 2.0
//    std::uniform_real_distribution<> freqFactorDist(-1.0, 1.0);  // Frequency factor between -1.0 and 1.0
//    std::uniform_real_distribution<> panDist(-1.0, 1.0);
//
//    // Generate noise notes with random parameters
//    for (int i = 0; i < numNotes; ++i)
//    {
//        // Generate random frequency factor and bandwidth
//        float currFreqFactor = freqFactorDist(gen);
//        float currBandwidth = bandwidthDist(gen);
//        float currPan = panDist(gen);
//
//        // Calculate the frequency using the random frequency factor
//        float frequency = std::pow(freqFactor, currFreqFactor) * offsetFactor;
//
//        // Create a NoiseNote with random parameters
//        relativeElevationPattern.emplace_back(frequency, currBandwidth, durationInSamples, currPan, harmonicEnvelope);
//    }
//
//    // Set the pattern with the generated noise notes
//    spatialPatternGenerator.setPattern(relativeElevationPattern);
    
    // Sandwich I
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 3000;
//    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
//    std::vector<float> bandwidths { 0.25, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> freqFactors { -1, -0.5, 0, 0.5, 1 };
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.1 }),
//    };
//
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    // Sandwich II
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 3000;
//    std::vector<NoiseNote> relativeElevationPattern {
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 0.1, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 1.0, { 1.0, 0.1 }),
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 0.1, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 0.1, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 1.0, { 1.0, 0.1 }),
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 0.1 }),
//        NoiseNote (1, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        
//    };
//
//    spatialPatternGenerator.setPattern (relativeElevationPattern);
    
    
    // Overlapping Patterns I
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 20000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth, durationInSamples, 0.0, { 1.0, 1.0 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Overlapping Patterns II
//    float bandwidth = 5.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 20000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples, 0.0, { 1.0, 1.0 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Overlapping Patterns III
//    float bandwidth = 5.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 20000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.5, 0.5 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Overlapping Patterns IV
//    float bandwidth = 1.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 40000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 1.0, 1.0 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Overlapping Patterns V
//    float bandwidth = 1.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Overlapping Patterns VI
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 2000;
//    std::vector<float> pans = { -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8,
//                                 1.0, 0.8, 0.6, 0.4, 0.2, 0.0, -0.2, -0.4, -0.6, -0.8 };
//    std::vector<NoiseNote> pattern1;
//    std::vector<NoiseNote> pattern2;
//    for (const auto& pan : pans)
//    {
//        pattern1.push_back (NoiseNote(std::pow(freqFactor, -1), bandwidth, durationInSamples, pan, { 1.0, 1.0 }));
//        pattern2.push_back (NoiseNote(std::pow(freqFactor, 1), bandwidth, durationInSamples, -pan, { 1.0, 1.0 }));
//    }
//
//    spatialPatternGenerator.setPattern (pattern1);
//    spatialPatternGenerator2.setPattern (pattern2);
    
    // Reference Intelligibility I
//    float bandwidth = 1.0;
//    int durationInSamples = 2000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(1, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }),
//        NoiseNote(1000, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false)
//    };
//    spatialPatternGenerator.setPattern (pattern1);
    
    // Reference Intelligibility II
//    float bandwidth = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(1, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }),
//        NoiseNote(1000, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false)
//    };
//    spatialPatternGenerator.setPattern (pattern1);
    
//    // Reference Intelligibility III
//    float bandwidth = 1.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0f, { 3.0, 3.0 }),
//        NoiseNote(1000, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//    };
//    
//    std::vector<NoiseNote> pattern2 {
//        NoiseNote(std::pow(freqFactor, -2), bandwidth, durationInSamples, 0.0f, { 3.0, 3.0 }),
//        NoiseNote(0, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//    };
//    spatialPatternGenerator.setPattern (pattern1);
//    spatialPatternGenerator2.setPattern (pattern2);
    
    // Noise Experiments
//    float bandwidth = 0.5;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(250, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//        NoiseNote(500, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//    };
//
//    std::vector<NoiseNote> pattern2 {
//        NoiseNote(1000, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//        NoiseNote(2000, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }, false),
//    };
//    spatialPatternGenerator.setPattern (pattern1);
//    spatialPatternGenerator2.setPattern (pattern2);
    
    // Reference Intelligibility IV
//    float bandwidth = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(1, bandwidth, durationInSamples, 0.0f, { 1.0, 1.0 }),
//        NoiseNote(2.0, bandwidth, durationInSamples, 0.0f, { 0.5, 1.0 })
//    };
//    spatialPatternGenerator.setPattern (pattern1);
    
    // X again
//    float bandwidth = 2.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> pattern1 {
//        NoiseNote(std::pow(freqFactor, -1), bandwidth, durationInSamples, -1.0f, { 1.0, 1.0 }),
//        NoiseNote(std::pow(freqFactor, 1), bandwidth, durationInSamples, 1.0f, { 0.5, 1.0 }),
//        NoiseNote(std::pow(freqFactor, -1), bandwidth, durationInSamples, 1.0f, { 0.5, 1.0 }),
//        NoiseNote(std::pow(freqFactor, 1), bandwidth, durationInSamples, -1.0f, { 0.5, 1.0 })
//    };
//    spatialPatternGenerator.setPattern (pattern1);
    
    // Noise on Noise I
//    float bandwidth = 1.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise II
//    float bandwidth = 1.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise II.V
//    float bandwidth = 100.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise III
//    float bandwidth = 1.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise III.V
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise III.V
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapPattern {
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 0.001, 0.001 })
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapPattern);
//    spatialPatternGenerator2.setPattern (hatPattern);
    
    // Noise on Noise IV
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise IV.V
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
//    // Noise on Noise V
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
////    // Noise on Noise VI
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    
//    // Noise on Noise VII
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
//    // Noise on Noise VII.V
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise 8
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples * 4, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples * 4, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples * 4, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (0, bandwidth, durationInSamples * 4, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    /*
    // Noise on Noise 9
    float bandwidth = 3.0;
    float freqFactor = 1.6;
    float offsetFactor = 1.0;
    int durationInSamples = 5000;
    std::vector<NoiseNote> clapLeftPattern {
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
    };
    std::vector<NoiseNote> hatLeftPattern {
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
    };
    
    std::vector<NoiseNote> clapRightPattern {
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
    };
    std::vector<NoiseNote> hatRightPattern {
        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
    };

    spatialPatternGenerator.setPattern (clapLeftPattern);
    spatialPatternGenerator2.setPattern (hatLeftPattern);
    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    */
    
    // Noise on Noise X
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise XI
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.2, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        
//    };
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.2, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise XII
//    float bandwidth = 3.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, 0, durationInSamples * 4, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, 0, durationInSamples * 4, 1.0, { 1.0, 1.0 }),
//       
//        
//    };
//    std::vector<NoiseNote> clapCenterPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatCenterPattern {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth * 0.5, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 10.0, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (0, 0, durationInSamples * 4, 0.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
//    spatialPatternGenerator5.setPattern (clapCenterPattern);
//    spatialPatternGenerator6.setPattern (hatCenterPattern);
    
    // Noise on Noise 11
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise 12
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, 1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.5, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
////        NoiseNote (0, bandwidth, durationInSamples, -1.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.5, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise VX
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 0.001, 0.001 })
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise VXI
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 })
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 })
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise VXII
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 100.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 5.0, 0.5 }),
//        NoiseNote (0, bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
//    // Noise on Noise VXIII
//    float bandwidth = 3.0;
//    float freqFactor = 1.6;
//    float offsetFactor = 1.0;
//    int durationInSamples = 40000;
//    std::vector<NoiseNote> clapLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, -1.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatLeftPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> clapRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 1.0, 3.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 1.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> hatRightPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 }),
//    };
//
//    spatialPatternGenerator.setPattern (clapLeftPattern);
//    spatialPatternGenerator2.setPattern (hatLeftPattern);
//    spatialPatternGenerator3.setPattern (clapRightPattern);
//    spatialPatternGenerator4.setPattern (hatRightPattern);
    
    // Noise on Noise VXIX
//    float bandwidth = 3.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (1, bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (1, bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise VXIX
//    float bandwidth = 3.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise XL
//    float bandwidth = 1.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, 3.5, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, 3.5, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise XL (mini)
//    float bandwidth = 0.5;
//    float freqFactor = 1.2;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise XLI
//    float bandwidth = 3.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
//    // Noise on Noise XLII
//    float bandwidth = 3.0;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (1, bandwidth * 0.5, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (1, bandwidth * 0.5, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise XLIII
//    float bandwidth = 4.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise XLIV
//    float bandwidth = 4.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 1.0, 4.0 }),
//        NoiseNote (1, bandwidth, durationInSamples, 0.0, { 4.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
    // Noise on Noise XLV
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 4.0 }),
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 3.0, 3.0 }),
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 4.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
    // Noise on Noise XLV
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 0.0, 6.0 }),
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 3.0, 3.0 }),
//        NoiseNote (1, bandwidth * 1.5, durationInSamples, 0.0, { 6.0, 0.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
//    // Noise on Noise XLVI
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 0.5, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.8, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise XLVII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.3), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.3), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2.6), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2.6), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.3, durationInSamples / 5.0, 0.55, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.3, durationInSamples / 5.0, -0.55, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.55, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.55, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.8, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.8, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise 2X
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3.0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3.0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.95, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise 2XI
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -0.5, 0.5 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
////    {
////        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 8.0, 0.5, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
////        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
////    };
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
//    // Noise on Noise 2XII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise 2XIII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 3.0, durationInSamples / 5.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 5.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise 2XIV
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise 2XV
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 0.1, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Noise on Noise 2XVI
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -0.5, 0.5 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth, durationInSamples / 4.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
    // Noise on Noise 2XVII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -0.5, 0.5 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
    // Noise on Noise 2XVIII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -0.5, 0.5 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
//    // Noise on Noise 2XIX
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, 0.0, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
//    // Noise on Noise 3X
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, 0.0, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 0.5, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 0.5, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 0.5, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 0.5, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 0.5, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
//    // Noise on Noise 3XI
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, 0.0, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
//    // Noise on Noise 3XII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 6000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.4, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.4, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.4, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.4, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.4, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.4, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.4, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.4, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.4, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // Noise on Noise 3XIII
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 6000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, -0.5, 0.0, 0.5, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // 3XIV calibration
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 6000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 1.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, 0.0, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.1, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // 2XS calibration
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.5, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3.0), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 3.0), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.5, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, 0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth * 0.3, durationInSamples / 5.0, -0.52, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, 0.54, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.0), bandwidth * 0.3, durationInSamples / 5.0, -0.54, { 1.0, 1.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, 0.95, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.5), bandwidth * 0.8, durationInSamples / 5.0, -0.95, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
//    spatialPatternGenerator4.setPattern (panPattern3);
    
    // XS Calibration
//    float bandwidth = 3.5;
//    float freqFactor = 1.5;
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples, 0.0, { 2.0, 2.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Y1 Calibration
//    float bandwidth = 3.0;
//    float freqFactor = 2.0;
//    int durationInSamples = 5000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 2.0, durationInSamples, 0.0, { 1.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1.5), bandwidth * 1.5, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 1.2, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 1.0, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.8, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<float> factors { 0.0 };
//    std::vector<float> pans { -1.0, 0.0, 1.0 };
//    std::vector<NoiseNote> panPattern2;
//    
//    for (const auto& pan : pans)
//    {
//        for (const auto& factor : factors)
//        {
//            panPattern2.push_back (NoiseNote (std::pow(freqFactor, factor), bandwidth * 3.0, durationInSamples * 2.0, pan, { 1.0, 1.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, -2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 2.5), bandwidth * 0.8, durationInSamples / 3.0, 0.0, { 1.0, 1.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
//    spatialPatternGenerator3.setPattern (panPattern2);
    
    
    // Dynamic XS Calibration
//    float bandwidth = 0.5;
//    float freqFactor = 1.2;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= ratio;
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
//    // Dynamic XS2 Calibration
//    float bandwidth = 0.5;
//    float freqFactor = 1.2;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::sqrt (ratio);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS3 Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS4 Calibration
//    float bandwidth = 1.0;
//    float freqFactor = 1.2;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS5 Calibration
//    float bandwidth = 1.5;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 6000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 0.5, durationInSamples / 2.0, -0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth * 0.5, durationInSamples / 2.0, 0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.5, durationInSamples / 2.0, -0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.5, durationInSamples / 2.0, 0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.5, durationInSamples / 2.0, -0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.5, durationInSamples / 2.0, 0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.5, durationInSamples / 2.0, -0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth * 0.5, durationInSamples / 2.0, 0.5, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
//    // Dynamic XS6 Calibration
//    float bandwidth = 1.5;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS7 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
////    spatialPatternGenerator.setPattern (noisePattern);
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS8 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> noisePattern {
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 1.0, 3.0 }),
//        NoiseNote (1, bandwidth * 3.0, durationInSamples, 0.0, { 3.0, 1.0 }),
//    };
//    std::vector<float> freqFactors { -1, 0, 1 };
//    std::vector<float> pans { -1, 0, 1 };
//    std::vector<NoiseNote> panPattern;
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            panPattern.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth, durationInSamples / 2.0, currPan, { 2.0, 2.0 }));
//            panPattern.push_back (NoiseNote (0, 0, durationInSamples / 2.0, currPan, { 2.0, 2.0 })); // silent note
//        }
//    }
//    spatialPatternGenerator2.setPattern (panPattern);
    
    // Dynamic XS9 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    int durationInSamples = 10000;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, durationInSamples / 2.0, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, durationInSamples, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, durationInSamples, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, durationInSamples, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Dynamic XSX Calibration (was just here)
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth *= ratio;
//        freqFactor *= std::pow (ratio, 1.0f / 3.0f);
//    }
//    
//    float timeRatio = 0.4;
//    int durationInSamples = 10000;
//    float breakDuration = (1 - timeRatio) * 3 * durationInSamples / 4.0;
//    float onDuration = timeRatio * 3 * durationInSamples / 2.0;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Dynamic F Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = ratio;
//        freqFactor = ratio;
//    }
//    
//    float timeRatio = 0.4;
//    int durationInSamples = 10000;
//    float breakDuration = (1 - timeRatio) * 3 * durationInSamples / 4.0;
//    float onDuration = timeRatio * 3 * durationInSamples / 2.0;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Dynamic F2 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2 * ratio;
//        freqFactor = ratio;
//    }
//    
//    float timeRatio = 0.4;
//    int durationInSamples = 10000;
//    float breakDuration = (1 - timeRatio) * 3 * durationInSamples / 4.0;
//    float onDuration = timeRatio * 3 * durationInSamples / 2.0;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Dynamic F3 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2.5 * ratio;
//        freqFactor = ratio;
//    }
//    
//    float timeRatio = 0.4;
//    int durationInSamples = 10000;
//    float breakDuration = (1 - timeRatio) * 3 * durationInSamples / 4.0;
//    float onDuration = timeRatio * 3 * durationInSamples / 2.0;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Dynamic F4 Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2.5 * ratio;
//        freqFactor = ratio;
//    }
//    
//    float timeRatio = 0.4;
//    int durationInSamples = 10000;
//    float breakDuration = (1 - timeRatio) * 3 * durationInSamples / 4.0;
//    float onDuration = timeRatio * 3 * durationInSamples / 2.0;
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.8, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.6, onDuration, -1.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration * 2.0, -1.0, { 2.0, 2.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.8, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.6, onDuration, 0.0, { 2.0, 2.0 }),
//        NoiseNote (0, 0, breakDuration, 0.0, { 2.0, 2.0 }),
//    };
//    
//    std::vector<NoiseNote> panPattern3 {
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, -1), bandwidth, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 0), bandwidth * 0.8, onDuration, 1.0, { 2.0, 2.0 }),
//        
//        NoiseNote (0, 0, breakDuration * 2.0, 1.0, { 2.0, 2.0 }),
//        NoiseNote (std::pow(freqFactor, 1), bandwidth * 0.6, onDuration, 1.0, { 2.0, 2.0 }),
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
    // Depth Calibration
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2.5 * ratio;
//        freqFactor = ratio;
//    }
//    
//    std::vector<float> ampls { -6.0, -3.0, 0.0 };
//    std::vector<float> pans { -1, 0, 1 };
//    std::vector<float> freqFactors { -1, 0, 1 };
//    std::vector<NoiseNote> panPattern;
//    int durationInSamples = 5000;
//    
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            for (const auto& currAmpl : ampls)
//            {
//                panPattern.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth, durationInSamples, currPan, { 2.0, 2.0 }, currAmpl));
//            }
//        }
//    }
//    spatialPatternGenerator.setPattern (panPattern);
    
//    // Depth Calibration 3
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2.5 * ratio;
//        freqFactor = ratio;
//    }
//    
//    std::vector<float> pans { -1, 0, 1, 0 };
//    std::vector<float> freqFactors { -1, 0, 1, 0 };
//    std::vector<NoiseNote> panPattern;
//    int durationInSamples = 5000;
//    
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            panPattern.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth, durationInSamples, currPan, { 2.0, 2.0 }, -3.0f));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern2;
//    for (const auto& currPan : pans)
//    {
//        panPattern2.push_back (NoiseNote (1.0f, bandwidth * 2.0, durationInSamples, currPan, { 1.0, 1.0 }, 0.0f));
//    }
//    
//    std::vector<NoiseNote> panPattern3;
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            panPattern3.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth / 2.0f, durationInSamples / 2.0f, currPan, { 2.0, 2.0 }, -6.0f));
//        }
//    }
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Flat Calibration 1
//    float bandwidth = 2.0;
//    float freqFactor = 1.1;
//    
//    auto lowerNode = amplCurve.nodeBelowFreq (freq);
//    auto higherNode = amplCurve.nodeAboveFreq (freq);
//    
//    if (lowerNode.has_value() && higherNode.has_value())
//    {
//        auto [lowerFreq, _] = lowerNode.value();
//        auto [higherFreq, __] = higherNode.value();
//        
//        float ratio = higherFreq / lowerFreq;
//        bandwidth = 2.5 * ratio;
//        freqFactor = ratio;
//    }
//    
//    std::vector<float> pans { -1, 0, 1, 0 };
//    std::vector<float> freqFactors { -1, 0, 1, 0 };
//    std::vector<NoiseNote> panPattern;
//    int durationInSamples = 5000;
//    
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            panPattern.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth, durationInSamples, currPan, { 2.0, 2.0 }));
//        }
//    }
//    
//    std::vector<NoiseNote> panPattern2;
//    for (const auto& currPan : pans)
//    {
//        panPattern2.push_back (NoiseNote (1.0f, bandwidth * 2.0, durationInSamples, currPan, { 1.0, 1.0 }));
//    }
//    
//    std::vector<NoiseNote> panPattern3;
//    for (const auto& currFreqFactor : freqFactors)
//    {
//        for (const auto& currPan : pans)
//        {
//            panPattern3.push_back (NoiseNote (std::pow(freqFactor, currFreqFactor), bandwidth / 2.0f, durationInSamples / 2.0f, currPan, { 2.0, 2.0 }));
//        }
//    }
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
//    spatialPatternGenerator3.setPattern (panPattern3);
    
//    // Diamond Calibration 1
//    float bandwidth = 2.0;
//    float freqFactor = 2.0;
//    
////    auto lowerNode = amplCurve.nodeBelowFreq (freq);
////    auto higherNode = amplCurve.nodeAboveFreq (freq);
////    
////    if (lowerNode.has_value() && higherNode.has_value())
////    {
////        auto [lowerFreq, _] = lowerNode.value();
////        auto [higherFreq, __] = higherNode.value();
////        
////        float ratio = higherFreq / lowerFreq;
////        bandwidth = 2.5 * ratio;
////        freqFactor = ratio;
////    }
//    
//    std::vector<float> pans { -1, 0, 1, 0 };
//    std::vector<float> freqFactors { -1, 0, 1, 0 };
//    int durationInSamples = 20000;
//    
//    std::vector<NoiseNote> panPattern {
//        NoiseNote (std::pow(freqFactor, -1.0), bandwidth, durationInSamples, 0.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 1.0), bandwidth, durationInSamples, 0.0, { 1.0, 1.0 })
//    };
//    
//    std::vector<NoiseNote> panPattern2 {
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, -1.0, { 1.0, 1.0 }),
//        NoiseNote (std::pow(freqFactor, 0.0), bandwidth, durationInSamples / 2.0, 1.0, { 1.0, 1.0 })
//    };
//    
//    spatialPatternGenerator.setPattern (panPattern);
//    spatialPatternGenerator2.setPattern (panPattern2);
    
    // PinkXL 1 Calibration
    float bandwidth = 1.0;
    float freqFactor = 1.5;
    int durationInSamples = 10000;
    std::vector<NoiseNote> noisePattern {
        NoiseNote (1, 3.5, durationInSamples, 0.0, { 1.0, 3.0 }),
        NoiseNote (1, 3.5, durationInSamples, 0.0, { 3.0, 1.0 }),
    };
    std::vector<NoiseNote> panPattern {
        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, -1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 0), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 1), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, -1.0, { 2.0, 2.0 }),
        NoiseNote (std::pow(freqFactor, 2), bandwidth, durationInSamples / 2.0, 1.0, { 2.0, 2.0 }),
    };
    
    spatialPatternGenerator.setPattern (noisePattern);
    spatialPatternGenerator2.setPattern (panPattern);
}

void PlaybackManager::updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
    spatialPatternGenerator3.setAmplCurve (amplCurve);
    spatialPatternGenerator4.setAmplCurve (amplCurve);
    spatialPatternGenerator5.setAmplCurve (amplCurve);
    spatialPatternGenerator6.setAmplCurve (amplCurve);
}

void PlaybackManager::startPanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::updatePanCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::startPhaseCalibration(float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::updatePhaseCalibration (float freq, Curve &amplCurve, Curve &panCurve, Curve &phaseCurve)
{
    // Taking a break
}

void PlaybackManager::startTestingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    auto ampl = amplCurve.valueAtFrequency (freq);
    auto pan = panCurve.valueAtFrequency (freq);
    auto phase = phaseCurve.valueAtFrequency (freq);
    SequenceableNote testingNote (freq, ampl, pan, phase, 2000);
    arbitrarySequencer5.setNotes ({ testingNote });
    
    //    std::cout << "start testing freq: " << freq << std::endl;
    //    sineSweepGenerator.updateSweep (freq, freq, amplCurve, panCurve, phaseCurve);
        
}

void PlaybackManager::updateTestingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
{
    auto ampl = amplCurve.valueAtFrequency (freq);
    auto pan = panCurve.valueAtFrequency (freq);
    auto phase = phaseCurve.valueAtFrequency (freq);
    SequenceableNote testingNote (freq, ampl, pan, phase, 2000);
    arbitrarySequencer5.updateNotes ({ testingNote });
}

//void PlaybackManager::startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
//{
//    sineSweepGenerator.setSweep (centerFreq, centerFreq, amplCurve, panCurve, phaseCurve);
//}
//
//void PlaybackManager::updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve, Curve phaseCurve)
//{
//    sineSweepGenerator.updateSweep (centerFreq, centerFreq, amplCurve, panCurve, phaseCurve);
//}

void PlaybackManager::stopTestingFreq()
{
    isTesting = false;
}

void PlaybackManager::setReferenceVolume (float volume)
{
    this->referenceVolume = volume;
}

void PlaybackManager::startPlayingReferenceFreqs()
{
//    int noteDurationInSamples = 5000;
//    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude, 0.0f, noteDurationInSamples);
//    SequenceableNote refNote2 (referenceNote2.frequency, referenceNote2.amplitude, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.setNotesForSpatialCalibration ({ refNote, refNote2 });
}

void PlaybackManager::updatePlayingReferenceFreqs()
{
//    int noteDurationInSamples = 5000;
//    SequenceableNote refNote (referenceNote.frequency, referenceNote.amplitude, 0.0f, noteDurationInSamples);
//    SequenceableNote refNote2 (referenceNote2.frequency, referenceNote2.amplitude, 0.0f, noteDurationInSamples);
//    arbitrarySequencer.updateNotesForSpatialCalibration({ refNote, refNote2 });
}

void PlaybackManager::setReferenceVolume1 (float volume)
{
    this->referenceNote.amplitude = volume + 6.0f;
}

void PlaybackManager::setReferenceVolume2 (float volume)
{
    this->referenceNote2.amplitude = volume + 6.0f;
}

void PlaybackManager::setReferencePan (float pan)
{
    this->referencePan = pan;
    this->leftRefNote.amplitude = referenceNote.amplitude - (pan < 0 ? pan : 0);
    this->rightRefNote.amplitude = referenceNote.amplitude + (pan > 0 ? pan : 0);
}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [leftSample1, rightSample1] = spatialPatternGenerator.getNextSample();
    auto [leftSample2, rightSample2] = spatialPatternGenerator2.getNextSample();
    auto [leftSample3, rightSample3] = spatialPatternGenerator3.getNextSample();
    auto [leftSample4, rightSample4] = spatialPatternGenerator4.getNextSample();
    auto [leftSample5, rightSample5] = spatialPatternGenerator5.getNextSample();
//    auto [leftSample6, rightSample6] = spatialPatternGenerator6.getNextSample();
    return { leftSample1 + leftSample2 + leftSample3 + leftSample4 + leftSample5, rightSample1 + rightSample2 + rightSample3 + rightSample4 + rightSample5 };
}

float PlaybackManager::getCompensationDBAtFrequency (float frequency)
{
    return 0.0f;//-4.5f * std::log2 (frequency / 1000.0f);
}

float PlaybackManager::getReferenceCompensationDBAtFrequency (float frequency)
{
    return 0.0f;
}
 
juce::dsp::IIR::Coefficients<float>::Ptr PlaybackManager::createDelayCoefficients(float sampleRate, float delaytime) const
{
    // Basic first order all pass filter
    float a = (1.0f - delaytime * 0.5f * sampleRate) / (1.0f + delaytime * 0.5f * sampleRate);
    juce::dsp::IIR::Coefficients<float>::Ptr coefs(new juce::dsp::IIR::Coefficients<float>(a * a, 2.0f * a, 1.0f, 1.0f, 2.0f * a, a * a));
    return coefs;
}

std::vector<SequenceableNote> PlaybackManager::getNotesForAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, bool alternateSilence, bool alternateSilenceBefore)
{
    float ampl = amplCurve.valueAtFrequency (freq);
    ampl += getCompensationDBAtFrequency (freq);
    
    float pan = panCurve.valueAtFrequency (freq);
    
    return getNotesForAmplCalibrationAt (freq, ampl, pan, alternateSilence, alternateSilenceBefore, true);
}

std::vector<SequenceableNote> PlaybackManager::getNotesForAmplCalibrationAt (float freq, float ampl, float pan, float phase, bool alternateSilence, bool alternateSilenceBefore, bool changeAmpl)
{
    // Play reference note and then do below - controlled - above
    int noteDurationInSamples = 1000;
    
    StereoGainEnvelope envelope (500);
    
    // Notes that will be used regardless
    SequenceableNote mainNote (freq, ampl, pan, phase, noteDurationInSamples, envelope);
    SequenceableNote silentNote (0.0f, 0.0f, 0.0f, 0.0f, noteDurationInSamples, StereoGainEnvelope::silent());
    
    std::vector<SequenceableNote> controlledNotes;
    if (alternateSilence && alternateSilenceBefore)
        controlledNotes.push_back (silentNote);
    controlledNotes.push_back (mainNote);
    if (alternateSilence && ! alternateSilenceBefore)
        controlledNotes.push_back (silentNote);
    
    // Play below - controlled - above while reference note is playing
    std::vector<SequenceableNote> notes;
    std::vector<float> pans { -1, -0.5, 0, 0.5, 1 };
    std::vector<float> ampls { 0.0, 20.0, -10.0, -20.0, 10.0 };
    
    int amplIdx = 0;
    for (const auto& pan : pans)
    {
        for (const auto& note : controlledNotes)
        {
            notes.emplace_back (note.withPan (pan).withAmplitudeChange (ampls[amplIdx]));
            if (changeAmpl)
                amplIdx++;
            if (amplIdx >= ampls.size())
                amplIdx = 0;
        }
    }
    
    
    return notes;
}


std::vector<SequenceableNote> PlaybackManager::getNotesForPanCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    float ampl = amplCurve.valueAtFrequency (freq);
    float pan = panCurve.valueAtFrequency (freq);
    float phase = phaseCurve.valueAtFrequency (freq);
    float noteDurationInSamples = 2000;
    StereoGainEnvelope envelope (500);
    
    std::vector<SequenceableNote> controlledNotes;
    std::vector<float> pans { -1, -0.5, 0.0, 0.5, 1 }; // 5 because it's a prime number
    std::vector<float> ampls { -6.0, 0.0, 6.0 }; // 3 because it's a prime number
    int numValues = 7;
    std::vector<float> phases(numValues);
    
    for (int i = 0; i < numValues; ++i)
    {
        phases[i] = -M_PI + i * (2 * M_PI / (numValues - 1)); // 7 because it's a prime number
    }
    
    SequenceableNote controlledNote (freq, ampl, pan, phase, noteDurationInSamples, envelope);
    
    int amplIdx = 0;
    int phaseIdx = 0;
    for (const auto& pan : pans)
    {
        controlledNotes.push_back (controlledNote.withPan (pan));//.withAmplitudeChange (ampls[amplIdx]).withPhase (phases[phaseIdx]));
        amplIdx++;
        phaseIdx++;
        if (amplIdx >= ampls.size())
            amplIdx = 0;
        if (phaseIdx >= phases.size())
            phaseIdx = 0;
    }
    
    return controlledNotes;
}
