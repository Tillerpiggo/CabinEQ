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
    float bandwidth = 0.5;
    float freqFactor = 1.15;
    
//    offsetFactor = 1.0;
    int durationInSamples = 20000;
    std::pair<float, float> harmonicEnvelope { 1.0, 1.0 };
    std::vector<NoiseNote> dynamicX2Pattern {
        NoiseNote (std::pow (freqFactor, -1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples, 0.0, harmonicEnvelope),
    };
    spatialPatternGenerator.setPattern (dynamicX2Pattern);
    
    // with context
    float contextFreqFactor = 2;
    std::vector<NoiseNote> contextNoisePattern {
        NoiseNote (std::pow (contextFreqFactor, -1), bandwidth, durationInSamples * 3, 0.0, harmonicEnvelope),
        NoiseNote (std::pow (freqFactor, 1), bandwidth, durationInSamples * 3, 0.0, harmonicEnvelope)
    };
//    spatialPatternGenerator2.setPattern (contextNoisePattern);
    
}

void PlaybackManager::updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve)
{
    spatialPatternGenerator.setAmplCurve (amplCurve);
    spatialPatternGenerator2.setAmplCurve (amplCurve);
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
    return { leftSample1 + leftSample2, rightSample1 + rightSample2 };
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
