/*
  ==============================================================================

    CutoffFilter.cpp
    Created: 27 Oct 2024 1:12:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CutoffFilter.h"

CutoffFilter::CutoffFilter()
{
    
}

void CutoffFilter::setCutoff (Type type, float freq)
{
    filterType = type;
    filterFreq = freq;
    switch (filterType)
    {
        case Type::lowPass:
            setWithLowPassCoefficients (filterFreq);
            break;
        case Type::highPass:
            setWithHighPassCoefficients (filterFreq);
            break;
    }
}

void CutoffFilter::process (juce::dsp::AudioBlock<float>& block)
{    
    juce::dsp::ProcessContextReplacing<float> context (block);
    cutoffFilter.process (context);
}

void CutoffFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    cutoffFilter.prepare (spec);
}

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = Filter::CoefficientsPtr;
void CutoffFilter::setWithLowPassCoefficients (float freq)
{
    std::cout << "sampleRate: " << sampleRate;
    setFilterCoefficients (juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(freq,
                                                                                                       sampleRate,
                                                                                                       8));
}

void CutoffFilter::setWithHighPassCoefficients (float freq)
{
    setFilterCoefficients (juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(freq,
                                                                                                       sampleRate,
                                                                                                       8));
}


