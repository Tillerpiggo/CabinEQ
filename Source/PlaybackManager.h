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
    
    void updateFilterWithCurves (Curve& amplCurve, Curve& panCurve); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    float getCurrPlayingFreq() const;
    float getCurrTestingFreq() const;
    float getCurrSineSweepFreq() const;
    
    void setIsTesting (bool isTesting);
    void setIsSweeping (bool isSweeping);
    void setIsCalibrating (bool isCalibrating);
    void setIsProcessing (bool isProcessing);
    void setDryWetVolumeBalance (float balance); // sets the dB balance between filter on/off
    
    void setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void startPlayingFreq (float freq, float ampl, float pan);
    void updatePlayingFreq (float freq, float ampl, float pan);
    // TODO: add diff functions for other kinds of tests
    void startTestingFreq (float freq, Curve& curve);
    void updateTestingFreq (float freq, Curve& curve);
    void stopTestingFreq();
    
    void setReferenceVolume (float volume);
    
    // Reference calibration
    void setReferencePan (float pan);
    
private:
    std::pair<float, float> getNextSample();
    float getCompensationDBAtFrequency (float frequency);
    float getReferenceCompensationDBAtFrequency (float frequency);
    juce::dsp::IIR::Coefficients<float>::Ptr createDelayCoefficients(float sampleRate, float delaytime) const;
    
    const int FFT_SIZE = 14;
    
    // Audio processing
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    // Sound generation
    ArbitrarySequencer arbitrarySequencer;
    ArbitrarySequencer arbitrarySequencer2;
    SineSweepGenerator sineSweepGenerator;
    Note referenceNote = Note (REFERENCE_FREQ, 6.0f, 0.0f);
    
    bool isTesting;
    bool isSweeping;
    bool isCalibrating;
    bool isProcessing;
    bool hasPreparedFilter;
    
    int currIdx = -1;
    int noteLength = 60000;
    
    float referenceVolume = 0.0f;
    float testingFreq = REFERENCE_FREQ;
    
    float referenceFreq = 1000.0f;
    float referencePan = 0.0f;
    
    Note leftRefNote { REFERENCE_FREQ, 6.0f, 0.0f };
    Note rightRefNote { REFERENCE_FREQ, 6.0f, 0.0f };
};
