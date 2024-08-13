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
    void setPhase (float phaseInDecibels) override;
    
private:
    void populateBuffer(); // fill heap block with next samples
    
    // Bandpass filter
    enum Slope
    {
        Slope_12,
        Slope_24,
        Slope_36,
        Slope_48
    };

    struct ChainSettings
    {
        float peakFreq { 0 }, peakGainInDecibels { 0 }, peakQuality { 1.f };
        float lowCutFreq { 500 }, highCutFreq { 700 };
        Slope lowCutSlope { Slope_12 }, highCutSlope { Slope_12 };
    };
    
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using BandpassFilter = juce::dsp::ProcessorChain<CutFilter, CutFilter>;
    
    BandpassFilter bandpass;
    
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, CoefficientType& coefficients)
    {
        updateCoefficients (chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }
    
    enum ChainPositions
    {
        LowCut,
        HighCut
    };
    
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& chain,
                         const CoefficientType& coefficients,
                         const Slope& slope)
    {
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        chain.template setBypassed<2>(true);
        chain.template setBypassed<3>(true);
        
        switch (slope)
        {
            case Slope_48:
            {
                update<3>(chain, coefficients);
            }
            case Slope_36:
            {
                update<2>(chain, coefficients);
            }
            case Slope_24:
            {
                update<1>(chain, coefficients);
            }
            case Slope_12:
            {
                update<0>(chain, coefficients);
            }
        }
    }
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    void updateFilters();
    
    // Pink noise generation
    PinkNoise pinkNoise;
    int bufferSize;
    int bufferIdx = 0;
    juce::AudioBuffer<float> buffer;
    juce::dsp::Gain<float> gainProcessor;
    
    float sampleRate = 44100;
    std::optional<Note> note;
};
