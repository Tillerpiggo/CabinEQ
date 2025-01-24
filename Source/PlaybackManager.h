/*
  ==============================================================================

    SliderCalibrationManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"
#include "FilterChain.h"
#include "GlyphGridPlayer.h"
#include "GridSequencer.h"
#include "ArbitraryResponseFilter.h"
#include "BandEqCurve.h"
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
    
    void setIsFilterOn (bool isFilterOn);
    void setIsPlayingNoise (bool isPlayingNoise);
    void setIsCabinNoise (bool isCabinNoise);
    void setVolume (float volume);
    void setCalibrationVolume (float calibrationVolume);
    void setMinFreq (float minFreq);
    void setMaxFreq (float maxFreq);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    
    void updateFIRFilter();
    void setIIR (bool isIIR);
    void setFIRQuality (int fftSize); // sets fftSize. DOESN'T UPDATE FIR FILTER AUTOMATICALLY!
    
    // Provisional bands
    void setProvisionalBands (std::vector<Band> provisionalBands);
    void setProvisionalBandsOn (bool isProvisionalOn);
    
    void setGlyphs (std::vector<Glyph> glyphs);
    void setGrid (NoiseSequenceGrid grid);
    float getCurrPlayingTime();
    std::vector<float> getCurrPlayingFreqs();
    
private:
    std::pair<float, float> getNextSample();
    
    BandEqCurve bandEqCurve;
    
    // Sound generation
    GridSequencer gridSequencer;
    GlyphGridPlayer glyphGridPlayer;
    
    // Audio Processing
    FilterChain filter;
    FilterChain provisionalFilter;
    ArbitraryResponseFilter firFilter;
    ArbitraryResponseFilter tiltFilter;
    Curve firCurve;
    TiltCurve tiltCurve;
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> profileVolumeProcessor;
    juce::dsp::Gain<float> overallVolumeProcessor;
    float volume = 0.0f; // in dB
    float calibrationVolume = 0.0f; // in dB
    
    // State
    bool isFilterOn; // if the EQ curve is being applied
    bool isPlayingNoise; // if calibration audio is being played rather than system audio
    bool isCabinNoise; // if it is, turn on the tilt filter
    bool isProvisionalOn = false; // if provisional bands are being applied to audio output
    bool isIIR = true; // if it is, use filterChain. Otherwise, use firFilter.
    
    int sampleCount = 0;
    int cycleTimeInSamples = 40000;
    float centerFreq = 1000.0f;
    float referenceFreq = 500.0f;
    
    int fftSize = 14;
};
