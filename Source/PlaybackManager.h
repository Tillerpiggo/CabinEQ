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
#include "RandomSineWaveGenerator.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithCurves (Curve& amplCurve, Curve& panCurve, int fftSize); // update the current filter with the curve
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    float getCurrPlayingFreq() const;
    float getCurrTestingFreq() const;
    float getCurrSineSweepFreq() const;
    
    void setIsTesting (bool isTesting);
    void setIsSweeping (bool isSweeping);
    void setIsCalibrating (bool isCalibrating);
    void setIsProcessing (bool isProcessing);
    void setDryWetVolumeBalance (float balance); // sets the dB balance between filter on/off
    void setWetVolume (float wetVolume);
    void setDryVolume (float dryVolume);
    
    void setSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void updateSineSweepCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void startPlayingFreq (float freq, Curve& amplCurve, Curve& panCurve);
    void updatePlayingFreq (float freq, Curve& amplCurve, Curve& panCurve);
    // TODO: add diff functions for other kinds of tests
    void startTestingFreq (float freq, Curve& curve);
    void updateTestingFreq (float freq, Curve& curve);
    void startSineSweep (float centerFreq, Curve amplCurve, Curve panCurve);
    void updateSineSweep (float centerFreq, Curve amplCurve, Curve panCurve);
    void stopTestingFreq();
    
    void setReferenceVolume (float volume);
    void setReferenceVolume1 (float volume);
    void setReferenceVolume2 (float volume);
    
    void startPlayingReferenceFreqs(); // TODO: Remove
    void updatePlayingReferenceFreqs(); // TODO: Remove
    
    // Reference calibration
    void setReferencePan (float pan);
    
private:
    std::pair<float, float> getNextSample();
    float getCompensationDBAtFrequency (float frequency);
    float getReferenceCompensationDBAtFrequency (float frequency);
    juce::dsp::IIR::Coefficients<float>::Ptr createDelayCoefficients(float sampleRate, float delaytime) const;
    std::vector<SequenceableNote> getNotesForCalibration (float freq, Curve& amplCurve, bool alternateSilence = false, bool alternateSilenceBefore = false);
    std::vector<SequenceableNote> getNotesForCalibrationAt (float freq, float ampl, bool alternateSilence = false, bool alternateSilenceBefore = false);
    
    const int FFT_SIZE = 15;
    
    // Audio processing
    ArbitraryResponseFilter filter;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    // Sound generation
    ArbitrarySequencer arbitrarySequencer;
    ArbitrarySequencer arbitrarySequencer2;
    ArbitrarySequencer arbitrarySequencer3;
    ArbitrarySequencer arbitrarySequencer4;
    ArbitrarySequencer arbitrarySequencer5;
    SineWaveGenerator sineWaveGenerator1;
    SineWaveGenerator sineWaveGenerator2;
    RandomSineWaveGenerator randomSineWaveGenerator;
    PinkNoise pinkNoise;
    SineSweepGenerator sineSweepGenerator;
    Note referenceNote = Note (REFERENCE_FREQ, 6.0f, 0.0f);
    Note referenceNote2 = Note (REFERENCE_FREQ_2, 6.0f, 0.0f);
    
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
    float referenceFreq2 = 5000.0f;
    float referencePan = 0.0f;
    
    Note leftRefNote { REFERENCE_FREQ, 6.0f, 0.0f };
    Note rightRefNote { REFERENCE_FREQ, 6.0f, 0.0f };
    
    Curve targetCurve;
};
