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
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithCurve (Curve& curve); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    float getCurrPlayingFreq() const;
    float getCurrTestingFreq() const;
    float getCurrSineSweepFreq() const;
    
    void setIsTesting (bool isTesting);
    void setIsSweeping (bool isSweeping);
    void setIsCalibrating (bool isCalibrating);
    void setIsBypassed (bool isBypassed);
    void setDryWetVolumeBalance (float balance); // sets the dB balance between filter on/off
    
    void setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void setCalibratingEQNode (EQNode node); // changes the EQNode being compared to the reference tone and restarts interval
    void updateCalibratingEQNode (EQNode updatingNode); // changes the EQNode being compared to the reference tone but does not restart the interval
    void startTestingFreq (float freq, Curve& curve);
    void updateTestingFreq (float freq, Curve& curve);
    void stopTestingFreq();
    
    void setReferenceVolume (float volume);
    
private:
    std::pair<float, float> getNextSample();
    float getCompensationDBAtFrequency (float frequency);
    float getReferenceCompensationDBAtFrequency (float frequency);
    
    const int FFT_SIZE = 18;
    
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    ArbitrarySequencer arbitrarySequencer;
    SineSweepGenerator sineSweepGenerator;
    Note referenceNote = Note (REFERENCE_FREQ, 6.0f, 0.0f, 0.0f);
    
    bool isTesting;
    bool isSweeping;
    bool isCalibrating;
    bool isBypassed;
    bool hasPreparedFilter;
    
    int currIdx = -1;
    int noteLength = 60000;
    
    float referenceVolume = 0.0f;
    float testingFreq = REFERENCE_FREQ;
    
    Curve targetCurve;
};
