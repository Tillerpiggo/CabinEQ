/*
  ==============================================================================

    FilterChain.h
    Created: 13 Oct 2024 1:06:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

//#pragma once
//
//using Filter = juce::dsp::IIR::Filter<float>;
//using Coefficients = juce::dsp::IIR::Coefficients<float>;
//
///// This manages a list of filters, making it easy to chain parametric bands to process audio
//class FilterChain
//{
//public:
//    void setBands (std::vector<Band> bands, double sampleRate)
//    {
//        std::cout << "setting bands" << std::endl;
//        this->filters.clear();
//        for (const auto& band : bands)
//        {
//            double Q = std::sqrt (std::pow (2, band.bandwidth)) / (std::pow (2, band.bandwidth) - 1);
//            addParametricBand (sampleRate, band.freq, Q, band.ampl);
//        }
//    }
//    
//    void addParametricBand (double sampleRate, double centerFreq, double qFactor, float amplInDB)
//    {
//        auto coefficients = Coefficients::makePeakFilter (sampleRate, centerFreq, qFactor, juce::Decibels::decibelsToGain (amplInDB));
//        addFilter (coefficients);
//    }
//    
//    void addFilter (const Coefficients::Ptr& coefficients)
//    {
//        auto filter = std::make_unique<Filter>();
//        filter->coefficients = coefficients;
//        filters.push_back (std::move (filter));
//    }
//    
//    void prepare (const juce::dsp::ProcessSpec& spec)
//    {
//        for (auto& filter : filters)
//        {
//            filter->prepare (spec);
//        }
//    }
//    
//    void process (juce::dsp::AudioBlock<float>& block)
//    {
////        auto numChannels = block.getNumChannels();
////            
////        for (size_t channel = 0; channel < numChannels; ++channel)
////        {
////            auto channelBlock = block.getSingleChannelBlock(channel);
////            juce::dsp::ProcessContextReplacing<float> context(channelBlock);
////            
////            for (auto& filter : filters)
////            {
////                filter->process(context);
////            }
////        }
//        juce::dsp::ProcessContextReplacing<float> context (block);
//        for (auto& filter : filters)
//        {
//            filter->process (context);
//        }
//    }
//    
//private:
//    std::vector<std::unique_ptr<Filter>> filters;
//};

#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <cmath>

using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

/// This manages a list of filters for stereo processing, with separate filters for left and right channels.
class FilterChain
{
public:
    void setBands (const std::vector<Band>& bands, double sampleRate)
    {
        this->bands = bands;
        this->sampleRate = sampleRate;
        
        for (int i = 0; i < bands.size(); ++i)
        {
            if (i >= filters.size())
            {
                Band band = bands[i];
                double Q = std::sqrt(std::pow(2.0, band.bandwidth)) / (std::pow(2.0, band.bandwidth) - 1);
                addParametricBand (filters, sampleRate, band.freq, Q, band.ampl);
            }
        }
        
        prepare (spec);
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        this->spec = spec;
        sampleRate = spec.sampleRate;
        // Prepare filters for the left channel
        for (auto& filter : filters)
        {
            filter->prepare(spec);
        }
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        // Update filters if needed before processing
        for (int i = 0; i < bands.size(); ++i)
        {
            Band band = bands[i];
            double Q = std::sqrt(std::pow(2.0, band.bandwidth)) / (std::pow(2.0, band.bandwidth) - 1);
            if (i < filters.size())
                updateParametricBand (filters, i, sampleRate, band.freq, Q, band.ampl);
        }

        // Process left channel through its filter chain
        for (auto& filter : filters)
        {
            filter->process (juce::dsp::ProcessContextReplacing (block));
        }
    }

private:
    std::vector<std::unique_ptr<Filter>> filters;
    std::vector<Band> bands;
    float sampleRate = 44100;
    
    juce::dsp::ProcessSpec spec;

    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        auto coefficients = Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                         juce::Decibels::decibelsToGain(amplInDB));

        addFilter (filters, coefficients);
    }
    
    
    void updateParametricBand (std::vector<std::unique_ptr<Filter>>& filters, int idx,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        auto coefficients = Coefficients::makePeakFilter (sampleRate, centerFreq, qFactor,
                                                         juce::Decibels::decibelsToGain(amplInDB));

        *filters[idx]->state = *coefficients;
    }
    
    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
                    const Coefficients::Ptr& coefficients)
    {
        auto filter = std::make_unique<Filter>();
        *filter->state = *coefficients;
        filter->prepare (spec);
        filters.push_back (std::move(filter));
    }
};
