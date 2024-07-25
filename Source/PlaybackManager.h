/*
  ==============================================================================

    SliderCalibrationManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SliderSequencer.h"
#include "SetPointManager.h"
#include "ArbitraryResponseFilter.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    int getCurrentlyPlayingIdx();
    
    void setSampleRate (float newSampleRate);
    void setIsCalibrating (bool isCalibrating);
    void setIsBypassed (bool isBypassed);
    void setDryWetVolumeBalance (float balance); // sets the dB balance between filter on/off
    
    void updateWithCurve (const Curve& curve); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
private:
    std::pair<float, float> getNextSample();
    
    const int FFT_SIZE = 12;
    
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    SliderSequencer sliderSequencer;
    
    bool isCalibrating;
    bool isBypassed;
    bool hasPreparedFilter;
    
    int currIdx = -1;
    int noteLength = 25000;
};
