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
#include "Channel.h"
#include "CrossfeedFilter.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithCurve (Curve& curve); // update the current filter with the curve
    void updateFilterWithCurves (Curve& leftCurve, Curve& rightCurve); // update the main filter with leftCurve and rightCurve (and crossfeed as well, for now)
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
    void setCalibratingEQNode (EQNode node, Channel channel); // changes the EQNode being compared to the reference tone and restarts interval
    void updateCalibratingEQNode (EQNode updatingNode, Channel channel); // changes the EQNode being compared to the reference tone but does not restart the interval
    void startTestingFreq (float freq, Curve& curve);
    void updateTestingFreq (float freq, Curve& curve);
    void stopTestingFreq();
    
    void setReferenceVolume (float volume);
    
    // Reference calibration
    void setReferenceFreq1 (float freq);
    void setReferenceFreq2 (float freq);
    void setReferenceAmplLeft1 (float ampl);
    void setReferenceAmplRight1 (float ampl);
    void setReferenceAmplLeft2 (float ampl);
    void setReferenceAmplRight2 (float ampl);
    
    
private:
    void setCrossfeed (Channel channel);
    std::pair<float, float> getNextSample();
    float getCompensationDBAtFrequency (float frequency);
    float getReferenceCompensationDBAtFrequency (float frequency);
    juce::dsp::IIR::Coefficients<float>::Ptr createDelayCoefficients(float sampleRate, float delaytime) const;
    
    const int FFT_SIZE = 14;
    
    // Audio processing
    ArbitraryResponseFilter filter;
    ArbitraryResponseFilter crossfeedFilter;
    CrossfeedFilter crossfeedFilterForCalibration;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    juce::dsp::AudioBlock<float> mainBlock, crossfeedBlock;
    juce::AudioBuffer<float> mainBuffer, crossfeedBuffer;
    
    // Sound generation
    ArbitrarySequencer arbitrarySequencer;
    ArbitrarySequencer arbitrarySequencer2;
    SineSweepGenerator sineSweepGenerator;
    Note referenceNote = Note (REFERENCE_FREQ, 6.0f);
    
    bool isTesting;
    bool isSweeping;
    bool isCalibrating;
    bool isProcessing;
    bool hasPreparedFilter;
    
    int currIdx = -1;
    int noteLength = 60000;
    
    float referenceVolume = 0.0f;
    float testingFreq = REFERENCE_FREQ;
    
    float referenceFreq1 = 800.0f;
    float referenceFreq2 = 2000.0f;
    float referenceAmplLeft1 = 0.0f;
    float referenceAmplRight1 = 0.0f;
    float referenceAmplLeft2 = 0.0f;
    float referenceAmplRight2 = 0.0f;
    float referenceCrossfeedGainLeft1 = 0.3f; // crossfeed while left is playing
    float referenceCrossfeedGainRight1 = 0.3f; // crossfeed while right is playing
    float referenceCrossfeedGainLeft2 = 0.3f; // crossfeed while left is playing
    float referenceCrossfeedGainRight2 = 0.3f; // crossfeed while right is playing
};
