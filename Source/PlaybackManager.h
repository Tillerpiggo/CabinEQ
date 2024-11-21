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
#include "ArbitraryResponseFilter.h"
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
    
    void setIsFilterOn (bool isFilterOn);
    void setIsPlayingNoise (bool isPlayingNoise);
    void setVolume (float volume);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    void setSizeFactor (float sizeFactor);
    void setCenterPos (juce::Point<float> centerPos);
    
    // Provisional bands
    void setProvisionalBands (std::vector<Band> provisionalBands);
    void setProvisionalBandsOn (bool isProvisionalOn);
    
    void setGlyph (Glyph glyph);
    float getCurrPlayingTime();
    
private:
    std::pair<float, float> getNextSample();
    
    // Audio processing
    GlyphGenerator glyphGenerator;
    FilterChain filter;
    FilterChain provisionalFilter;
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> profileVolumeProcessor;
    juce::dsp::Gain<float> overallVolumeProcessor;
    float volume = 0.0f; // in dB
    float calibrationVolume = 0.0f; // in dB
    
    // Sound generation
    ArbitraryResponseFilter tiltFilter; // to make the pink noise into Cabin Noise
    Curve tiltCurve;
    
    // State
    bool isFilterOn; // if the EQ curve is being applied
    bool isPlayingNoise; // if calibration audio is being played rather than system audio
    bool isProvisionalOn = false; // if provisional bands are being applied to audio output
};
