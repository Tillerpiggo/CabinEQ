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
#include "QualityStep.h"
#include "SequencerListener.h"
#include <random>

/// This class manages the playback of audio in the app, providing an interface for the PluginProcessor to easily
/// process audio or play sine tones as needed.
class PlaybackManager  : public SequencerListener
{
public:
    PlaybackManager();

    void processBlock (juce::AudioBuffer<float>& buffer);
    
    void updateFilterWithBandProfile (BandProfile bandProfile);
    void prepare (const juce::dsp::ProcessSpec& spec);
    
    void setIsProcessing (bool isFilterProcessing);
    void setIsCalibrating (bool isCalibrating);
    void setIsCycling (bool isCycling);
    void setVolume (float volume);
    void setMelodicPattern (MelodicNotes melodicNotes);
    void setSpatialPattern (Glyph glyph);
    
    float getCurrPlayingFreq();
    int getStage() const;
    
    void setQualityStep (QualityStep qualityStep);
    int getStage();
    void setStage (int stageIdx);
    void onCycleFinish() override;
    void setListener (SequencerListener* listener);
    
private:
    std::pair<float, float> getNextSample();
    void updateSequencersFromQualityStep();
    
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
    
    // State
    std::optional<QualityStep> qualityStep;
    int stageIdx = 0;
    
    bool isProcessing;
    bool isCalibrating;
    bool isCycling;
    SequencerListener* listener;
};
