/*
  ==============================================================================

    SliderCalibrationManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitraryResponseFilter.h"
#include "ArbitrarySequencer.h"
#include "SineSweepGenerator.h"
#include "PinkNoiseGenerator.h"
#include "Constants.h"
#include "RandomSineWaveGenerator.h"
#include "SpatialPatternGenerator.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithCurves (Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, int fftSize); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    float getCurrPlayingFreq() const;
    float getCurrProbingFreq() const;
    float getCurrSineSweepFreq() const;
    
    void setIsTesting (bool isTesting);
    void setIsSweeping (bool isSweeping);
    void setIsCalibrating (bool isCalibrating);
    void setIsProcessing (bool isProcessing);
    void setWetVolume (float wetVolume);
    void setDryVolume (float dryVolume);
    
    void startAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void updateAmplCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void startPanCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void updatePanCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void startPhaseCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void updatePhaseCalibration (float freq, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve);
    void startProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve);
    void updateProbingFreq (float freq, Curve amplCurve, Curve panCurve, Curve phaseCurve);
    void stopProbing();
    
    void setReferenceVolume (float volume);
    void setReferenceVolume1 (float volume);
    void setReferenceVolume2 (float volume);
    
    // Reference calibration
    void setReferencePan (float pan);
    
private:
    std::pair<float, float> getNextSample();
    
    const int FFT_SIZE = 15;
    
    // Audio processing
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> gainProcessor;
    float wetVolume = 0.0f; // in dB
    float dryVolume = 0.0f; // in dB
    
    // Sound generation
    SpatialPatternGenerator spatialPatternGenerator;
    SpatialPatternGenerator spatialPatternGenerator2;
    SpatialPatternGenerator spatialPatternGenerator3;
    SpatialPatternGenerator spatialPatternGenerator4;
    SpatialPatternGenerator spatialPatternGenerator5;
    SpatialPatternGenerator spatialPatternGenerator6;
    ArbitrarySequencer arbitrarySequencer;
    ArbitrarySequencer arbitrarySequencer2;
    ArbitrarySequencer arbitrarySequencer3;
    ArbitrarySequencer arbitrarySequencer4;
    ArbitrarySequencer arbitrarySequencer5;
    
    bool isCalibrating;
    bool isProbing;
    bool isProcessing;
    bool hasPreparedFilter;
    
    float referenceVolume = 0.0f;
    float probingFreq = REFERENCE_FREQ;
};
