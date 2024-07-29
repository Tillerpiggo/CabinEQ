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
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithCurve (const Curve& curve); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setIsCalibrating (bool isCalibrating);
    void setIsBypassed (bool isBypassed);
    void setDryWetVolumeBalance (float balance); // sets the dB balance between filter on/off
    
    void setCalibratingEQNode (EQNode calibratingNode); // changes the EQNode being compared to the reference tone and restarts interval
    void updateCalibratingEQNode (EQNode updatedNode); // changes the EQNode being compared to the reference tone but does not restart the interval
    
private:
    std::pair<float, float> getNextSample();
    float getCompensationDBAtFrequency (float frequency);
    
    const int FFT_SIZE = 13;
    
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    ArbitrarySequencer arbitrarySequencer;
    Note referenceNote = Note (1000.0f, 6.0f, 0.0f, 0.0f);
    
    bool isCalibrating;
    bool isBypassed;
    bool hasPreparedFilter;
    
    int currIdx = -1;
    int noteLength = 25000;
};
