/*
  ==============================================================================

    SpatialPatternGenerator.h
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialNoiseGenerator.h"
#include "StereoGainEnvelope.h"

struct NoiseNote
{
    NoiseNote (float freqFactor, float bandwidth, int durationInSamples, float pan, std::pair<float, float> bandwidthEnvelope, StereoGainEnvelope envelope = StereoGainEnvelope::clap(), bool isRelativeToCenterFrequency = true)
    : freqFactor (freqFactor), bandwidth (bandwidth), durationInSamples (durationInSamples), pan (pan), envelope (envelope),
      bandwidthEnvelope (bandwidthEnvelope),
      isRelativeToCenterFrequency (isRelativeToCenterFrequency)
    {}
    
    float freqFactor; // frequency factor from center frequency
    float bandwidth; // bandwidth in octaves
    int durationInSamples; // how long it lasts
    float pan; // the panning, from -1 to 1, of the noise note
    StereoGainEnvelope envelope;
    std::pair<float, float> bandwidthEnvelope; // first = headFactor, second = tailFactor. { 0.3, 1.5 } would indicate a long head and short tail.
    bool isRelativeToCenterFrequency;
    
    std::pair<float, float> getGainAtSample (int sample)
    {
        return envelope.getGainAtSample (sample, durationInSamples);
    }
    
    
    NoiseNote withPanChange (float panChange)
    {
        return NoiseNote (freqFactor, bandwidth, durationInSamples, pan + panChange, bandwidthEnvelope, envelope,  isRelativeToCenterFrequency);
    }
};

class SpatialPatternGenerator {
public:
    SpatialPatternGenerator();
    
    void setSampleRate (float sampleRate);
    void setPattern (std::vector<NoiseNote> notes);
    void setCenterFrequency (float centerFrequency);
    void setAmplCurve (Curve& amplCurve);
    std::pair<float, float> getNextSample();

private:
    NoiseNote getCurrNote();
    void updateBandpassAndPanning();
    void goToNextNote();

    SpatialNoiseGenerator noiseGenerator;
    int numSamplesNoteHasBeenPlaying;
    int currNoteIdx;
    float centerFrequency;
    float leftGain;
    float rightGain;
    
    std::vector<NoiseNote> notes;
};

