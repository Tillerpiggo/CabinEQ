/*
  ==============================================================================

    SpatialPatternGenerator.h
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialPinkNoiseGenerator.h"
#include "StereoGainEnvelope.h"
#include "SequencerListener.h"

struct NoiseNote
{
    NoiseNote (float freqFactor, float bandwidth, int durationInSamples, float pan, std::pair<float, float> bandwidthEnvelope, bool isRelativeToCenterFrequency = true, float ampl = 0.0, StereoGainEnvelope envelope = StereoGainEnvelope::clap())
    : freqFactor (freqFactor), bandwidth (bandwidth), durationInSamples (durationInSamples), pan (pan), envelope (envelope),
      bandwidthEnvelope (bandwidthEnvelope),
      isRelativeToCenterFrequency (isRelativeToCenterFrequency),
      ampl (ampl)
    {}
    
    float freqFactor; // frequency factor from center frequency
    float bandwidth; // bandwidth in octaves
    int durationInSamples; // how long it lasts
    float pan; // the panning, from -1 to 1, of the noise note
    StereoGainEnvelope envelope;
    std::pair<float, float> bandwidthEnvelope; // first = headFactor, second = tailFactor. { 0.3, 1.5 } would indicate a long head and short tail.
    bool isRelativeToCenterFrequency;
    float ampl; // in dB
    
    std::pair<float, float> getGainAtSample (int sample)
    {
        return envelope.getGainAtSample (sample, durationInSamples);
    }
    
    
    NoiseNote withPanChange (float panChange)
    {
        return NoiseNote (freqFactor, bandwidth, durationInSamples, pan + panChange, bandwidthEnvelope, isRelativeToCenterFrequency, ampl, envelope);
    }
};

class SpatialPatternGenerator {
public:
    SpatialPatternGenerator();
    
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setPattern (const std::vector<NoiseNote> notes);
    void setPeakFilter (float centerFreq, float bandwidth, float ampl);
    void setLowCutFilter (float freq);
    void setMelodicPattern (std::vector<int> notesInSemitones, float centerFreq, float bandwidth, float noteDurationInMs);
    void setMelodicPattern (std::vector<int> notesInSemitones, std::vector<float> pans, float centerFreq, float bandwidth, float noteDurationInMs); // assumes that len(notesInSemitones) == len(pans). Pans should be from [-1, 1]
    void setCenterFrequency (float centerFrequency);
    void setNoteCenterFreq (float noteCenterFreq); // change the center freq of all notes being played
    void setNoteBandwidth (float noteBandwidth); // change the bandwidth of all notes being played (with non-zero bandwidth)
    std::pair<float, float> getNextSample();
    
    void setSpeedFactor (float speedFactor);
    void setFreqFactor (float freqFactor);
    void setListener (SequencerListener* listener);
    std::optional<float> getCurrPlayingFreq();

private:
    NoiseNote getCurrNote();
    void updateBandpassAndPanning();
    void goToNextNote();

    SpatialPinkNoiseGenerator noiseGenerator;
    int numSamplesNoteHasBeenPlaying;
    int currNoteIdx;
    float centerFrequency;
    float leftGain;
    float rightGain;
    float sampleRate;
    float speedFactor = 1.0f;
    float freqFactor = 1.0f;
    
    juce::dsp::IIR::Filter<float> leftPeakFilter;
    juce::dsp::IIR::Filter<float> rightPeakFilter;
    
    std::vector<NoiseNote> notes;
    
    // For dynamic changes
    std::optional<float> noteCenterFreq; // will override all notes center freq if set
    std::optional<float> noteBandwidth; // will override all notes (with non-0 bandwidth) if set
    
    SequencerListener* listener = nullptr;
    
};
