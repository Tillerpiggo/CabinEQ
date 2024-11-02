/*
  ==============================================================================

    MelodicNoiseSequencer.h
    Created: 27 Oct 2024 1:05:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PinkNoise.h"
#include "SpatialPatternGenerator.h" // for NoiseNote
#include "SpatialPinkNoiseGenerator.h"
#include "SineWaveGenerator.h"
#include "SequencerListener.h"

class MelodicNoiseSequencer
{
public:
    MelodicNoiseSequencer();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (std::vector<NoiseNote> notes, float relativeNoiseGain = 1.0f);
    void setMelodyVolume (float melodyVolume);
    void setSineVolume (float sineVolume);
    void setSpeedFactor (float speedFactor);
    void setOctaveRange (float octaveRange);
    std::pair<float, float> getNextSample();
    
    void mute();
    void setListener (SequencerListener* listener);
    
private:
    NoiseNote getCurrNote();
    void updateFilters();
    void updatePeakFilter();
    void goToNextNote();
    
    std::vector<NoiseNote> notes;
    
    PinkNoise pinkNoise;
    SineWaveGenerator sineWaveGenerator;
    
    int numSamplesNoteHasBeenPlaying;
    int currNoteIdx;
    float sampleRate;
    bool isMuted = false;
    float relativeNoiseGain = 1.0f;
    
    juce::dsp::IIR::Filter<float> peakFilter;
//    juce::dsp::IIR::Filter<float> notchFilter; // to add a notch in the main noise
//    juce::dsp::IIR::Filter<float> bandpassFilter;
//    juce::dsp::IIR::Filter<float> lowPassFilter;
//    juce::dsp::IIR::Filter<float> highPassFilter;
    int snapToZeroCounter = 0;
    float melodyVolume = 0.0f; // in db
    float sineVolume = 3.0f;
    float speedFactor = 1.0f;
    float freqOffsetFactor = 1.0f;
    float octaveRange = 3.0f; // +- octaves of randomization
    
    SequencerListener* listener;
};
