/*
  ==============================================================================

    PinkNoiseGenerator.h
    Created: 12 Aug 2024 8:56:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PitchedGenerator.h"
#include "PinkNoise.h"

// Generates banded pink noise at a certain frequency, volume, pan, and phase.
class PinkNoiseGenerator   : public PitchedGenerator
{
public:
    PinkNoiseGenerator();
    
    void setSampleRate (float newSampleRate) override;
    const std::pair<float, float> getNextSample() override;
    
    void setNote (Note note) override;
    void setFrequency (float frequencyInHz) override;
    void setVolume (float volumeInDecibels) override;
    void setPan (float panInDecibels) override;
    
private:
    void populateBuffer(); // fill heap block with next samples
    void updateAmplitudeCompensation();
    
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter>;
    using BandpassFilter = juce::dsp::ProcessorChain<CutFilter, CutFilter>;
    using Coefficients = Filter::CoefficientsPtr;
    
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter (ChainType& chain, const CoefficientType& coefficients);
    template<int Index, typename ChainType, typename CoefficientType>
    void update (ChainType& chain, CoefficientType& coefficients);
    void updateBandpassFilter (const float lowCutFreq, const float highCutFreq);
    static void updateCoefficients (Coefficients& old, const Coefficients& replacements);
    
    // Pink noise generation
    PinkNoise pinkNoise;
    juce::Random noiseSrc;
    BandpassFilter bandpass;
    
    float centerFrequency = 1000.0f;
    float volumeInDB = 0.0f;
    float panInDB = 0.0f;
    float leftAmplitudeCompensation = 0.0f;
    float rightAmplitudeCompensation = 0.0f;
    int bufferSize;
    int bufferIdx = 0;
    juce::AudioBuffer<float> buffer;
    juce::dsp::Gain<float> gainProcessor;
    
    float sampleRate = 44100;
    
    enum ChainPositions
    {
        LowCut,
        HighCut
    };
};
