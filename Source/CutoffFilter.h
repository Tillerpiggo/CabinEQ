/*
  ==============================================================================

    CutoffFilter.h
    Created: 27 Oct 2024 1:12:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CutoffFilter
{
public:
    enum class Type
    {
        lowPass,
        highPass,
    };
    
    CutoffFilter();
    void process (juce::dsp::AudioBlock<float>& block);
    void prepare (const juce::dsp::ProcessSpec& spec);
    void setCutoff (Type type, float freq);
    
private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = Filter::CoefficientsPtr;
    
    // Cascade 8 cut filters
    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter> cutoffFilter;
    float sampleRate;
    
    void setWithLowPassCoefficients (float freq);
    void setWithHighPassCoefficients (float freq);
    
    template<typename CoefficientType>
    void setFilterCoefficients (const CoefficientType& coefficients)
    {
        update<0>(coefficients);
        update<1>(coefficients);
        update<2>(coefficients);
        update<3>(coefficients);
        update<4>(coefficients);
        update<5>(coefficients);
        update<6>(coefficients);
        update<7>(coefficients);
    }
    
    template<int Index, typename CoefficientType>
    void update (CoefficientType& coefficients)
    {
        *cutoffFilter.template get<Index>().coefficients = *coefficients[Index];
    }
    
    Type filterType;
    float filterFreq = 1000.0f;
    bool shouldUpdateFilter = false;
};
