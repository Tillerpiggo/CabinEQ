/*
  ==============================================================================

    GlyphGridPlayer.h
    Created: 23 Dec 2024 3:58:50pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"
#include "GainEnvelope.h"
#include "PatternEnvelope.h"
#include "NoiseGenerator.h"
#include "ArbitraryResponseFilter.h"

// This plays a grid of glyphs simultaneously
class GlyphGridPlayer
{
public:
    GlyphGridPlayer();
    
    std::pair<float, float> getNextSample();
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setGlyphs (std::vector<Glyph> glyphs);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    void setFrequencyRange (float minFreq, float maxFreq);
    void setMinFreq (float minFreq);
    void setMaxFreq (float maxFreq);
    void setPanRange (float leftmostPan, float rightmostPan); // max leftmostPan is -1 and max rightmostPan is 1
    void setBarkScaling (bool barkScalingEnabled);
    void setERBScaling (bool erbScalingEnabled);
    
    void setIsCascading (bool isCascading);
    void setDensity (int density);
    void setStrokeOverlap (float strokeOverlap);
    void setDotOverlap (float dotOverlap);
    void setRampLength (float rampLength);
    
    float getCurrPlayingTime();
    
    std::vector<float> getCurrPlayingFreqs();
    std::vector<std::pair<float, float>> getCurrPlayingFreqsAndVols();
    
    void processBlock(juce::AudioBuffer<float>& buffer, float gain = 1.0f);
    
private:
    void addRemoveNoiseGeneratorsIfNeeded();
    void updateNoiseGeneratorsIfNeeded();
    float scaleToBarkBandwidth (float bandwidth, float centerFrequency);
    float barkToHz (float hz);
    
    std::pair<std::pair<float, float>, float> getFreqPanVolFromGlyphAtTime (const Glyph& glyph, float currTime);
    std::vector<std::pair<std::pair<float, float>, float>> getFreqPanVolsFromGlyphAtTime (const Glyph& glyph, float currTime);
    float volToDB (float vol); // converts volume (from 0 to 1) to db using some arbitrary formula
    
    juce::dsp::ProcessSpec spec;
    std::vector<Glyph> glyphs;
    
    std::vector<NoiseGenerator> noiseGenerators;
    GainEnvelope gainEnvelope { 1000 };
    float currTime = 0.0f;
    float timeInterval = 0.0f; // must be set in prepare first
    
    // Settings
    float speedFactor = 1.0f;
    float bandwidth = 1.0f;
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;
    float leftmostPan = -1.0f;
    float rightmostPan = 1.0f;
    
    // Counters
    int updateBandpassCounter = 0;
    int samplesUntilBandpassUpdate = 500;
    
    bool shouldAddRemoveNoiseGenerators = false;
    bool shouldUpdateNoiseGenerators = false;
    
    bool barkScalingEnabled = false;
    bool erbScalingEnabled = false;
    
    PatternEnvelope patternEnvelope;

    ArbitraryResponseFilter tiltFilter;
    TiltCurve tiltCurve;
};
