/*
  ==============================================================================

    SliderCalibrationManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoiseGenerator.h"
#include "SpatialPinkNoiseGenerator.h"
#include "WrapperPinkNoiseGenerator.h"
#include "Constants.h"
#include "SpatialPatternGenerator.h"
#include "MelodicNotes.h"
#include "BandProfile.h"
#include "FilterChain.h"
#include "ArbitraryResponseFilter.h"
#include "ArbitrarySequencer.h"
#include "SineSweepGenerator.h"
#include "NoiseSweepGenerator.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithBandProfile (BandProfile bandProfile);
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setIsTesting (bool isTesting);
    void setIsCalibrating (bool isCalibrating);
    void setIsProcessing (bool isProcessing);
    void setVolume (float volume);
    
    void startAmplCalibration (float freq, float bandwidth);
    void updateAmplCalibration (float freq, float bandwidth);
    void setPatternSolo (bool solo);
    
    void setMutedGenerators (std::vector<bool> mutedGens); // takes a vector of length 4 with bools for if each generate is muted (true) or not (false)
    
    void setReferenceVolume (float volume);
    void setReferenceVolume1 (float volume);
    void setReferenceVolume2 (float volume);
    
    // Reference calibration
    void setReferencePan (float pan);
    
    float getCurrPlayingFreq();
    
private:
    std::pair<float, float> getNextSample();
    
    // Audio processing
    FilterChain filter;
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> profileVolumeProcessor;
    juce::dsp::Gain<float> overallVolumeProcessor;
    float volume = 0.0f; // in dB
    
    // Sound generation
    WrapperPinkNoiseGenerator wrapperPinkNoiseGenerator;
    SpatialPatternGenerator spatialPatternGenerator;
    SpatialPatternGenerator spatialPatternGenerator2;
    SpatialPatternGenerator spatialPatternGenerator3;
    SpatialPatternGenerator spatialPatternGenerator4;
    SpatialPatternGenerator spatialPatternGenerator5;
    SpatialPatternGenerator spatialPatternGenerator6;
    SpatialPinkNoiseGenerator spatialPinkNoiseGenerator;
    NoiseSweepGenerator noiseSweepGenerator;
    NoiseSweepGenerator noiseSweepGenerator2;
    NoiseSweepGenerator noiseSweepGenerator3;
    NoiseSweepGenerator noiseSweepGenerator4;
    ArbitrarySequencer arbitrarySequencer;
    ArbitrarySequencer arbitrarySequencer2;
    ArbitrarySequencer arbitrarySequencer3;
    ArbitraryResponseFilter tiltFilter; // to make the pink noise into Cabin Noise
    Curve tiltCurve;
    
    // Sweeps
    SineSweepGenerator sineSweepGenerator;
    
    bool isCalibrating;
    bool isProcessing;
    bool hasPreparedFilter;
    
    float referenceVolume = 0.0f;
    
    bool isSpatialPatternGeneratorMuted = false;
    bool isSpatialPatternGenerator2Muted = false;
    bool isSpatialPatternGenerator3Muted = false;
    bool isSpatialPatternGenerator4Muted = false;
};
