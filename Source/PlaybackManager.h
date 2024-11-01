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
#include "SpatialPatternGenerator.h"
#include "MelodicNotes.h"
#include "BandProfile.h"
#include "FilterChain.h"
#include "ArbitraryResponseFilter.h"
#include "ArbitrarySequencer.h"
#include "NoiseSweepGenerator.h"
#include "MelodicNoiseSequencer.h"
#include "GlyphGenerator.h"
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
    
    void setIsProcessing (bool isFilterProcessing);
    void setIsCalibrating (bool isCalibrating);
    void setVolume (float volume);
    
    float getCurrPlayingFreq();
    
private:
    std::pair<float, float> getNextSample();
    
    // Audio processing
    FilterChain filter;
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> profileVolumeProcessor;
    juce::dsp::Gain<float> overallVolumeProcessor;
    float volume = 0.0f; // in dB
    float calibrationVolume = 0.0f; // in dB
    float spacing = 3.0f; // in octaves
    
    // Sound generation
    GlyphGenerator glyphGenerator;
    MelodicNoiseSequencer melodicNoiseSequencer;
    ArbitraryResponseFilter tiltFilter; // to make the pink noise into Cabin Noise
    Curve tiltCurve;
    
    bool isProcessing;
    bool isCalibrating;
};
