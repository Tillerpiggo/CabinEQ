///*
//  ==============================================================================
//
//    CutoffFilter.h
//    Created: 27 Oct 2024 1:12:36pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#pragma once
//
//#include <JuceHeader.h>
//
//class CutoffFilter
//{
//public:
//    enum class CutoffType
//    {
//        lowPass,
//        highPass,
//    };
//    
//    CutoffFilter();
//    void process (juce::dsp::AudioBlock<float>& block);
//    
//    void setCutoff (CutoffType type, float freq);
//    
//private:
//    using Filter = juce::dsp::IIR::Filter<float>;
//    using Coefficients = Filter::CoefficientsPtr;
//    
//    // Cascade 8 cut filters
//    juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter> cutoffFilter;
//    
//    void setWithLowPassCoefficients (float freq);
//    void setWithHighPassCoefficients (float freq);
//    void setFilterCoefficients (Filter::CoefficientsPtr coefficients);
//    CutoffType filterType;
//    float filterFreq;
//    bool shouldUpdateFilter = false;
//};
