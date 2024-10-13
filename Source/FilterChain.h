/*
  ==============================================================================

    FilterChain.h
    Created: 13 Oct 2024 1:06:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

/// This manages a list of filters, making it easy to chain parametric bands to process audio
class FilterChain
{
public:
    void addParametricBand (double centerFreq, double qFactor, float amplInDB)
    {
        auto coefficients = Coefficients::makePeakFilter (sampleRate, centerFreq, qFactor, juce::Decibels::decibelsToGain (amplInDB));
        addFilter (coefficients);
    }
    
    void addFilter (const Coefficients::Ptr& coefficients)
    {
        auto filter = std::make_unique<Filter>();
        filter->coefficients = coefficients;
        filters.push_back (std::move (filter));
    }
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        for (auto& filter : filters)
        {
            filter->prepare (spec);
        }
    }
    
    void process (juce::dsp::AudioBlock<float>& block)
    {
        juce::dsp::ProcessContextReplacing<float> context (block);
        for (auto& filter : filters)
        {
            filter->process (context);
        }
    }
    
private:
    std::vector<std::unique_ptr<Filter>> filters;
};
