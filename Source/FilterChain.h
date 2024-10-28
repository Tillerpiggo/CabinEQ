/*
  ==============================================================================

    FilterChain.h
    Created: 13 Oct 2024 1:06:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

//#pragma once
//
//#include <vector>
//#include <memory>
//#include <iostream>
//#include <cmath>
//
//using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
//using Coefficients = juce::dsp::IIR::Coefficients<float>;
//
///// This manages a list of filters for stereo processing, with separate filters for left and right channels.
//class FilterChain
//{
//public:
//    void setBands (const std::vector<Band>& bands, double sampleRate)
//    {
//        this->bands = bands;
//        this->sampleRate = sampleRate;
//        this->shouldUpdateFilters = true;
//        
//        bool didAddBands = false;
//        if (bands.size() > filters.size())
//        {
//            for (int i = 0; i < bands.size(); ++i)
//            {
//                if (i >= filters.size())
//                {
//                    Band band = bands[i];
//                    double Q = std::sqrt(std::pow(2.0, band.bandwidth)) / (std::pow(2.0, band.bandwidth) - 1);
//                    addParametricBand (filters, sampleRate, band.freq, Q, band.ampl);
//                    didAddBands = true;
//                }
//            }
//        }
//    }
//
//    void prepare (const juce::dsp::ProcessSpec& spec)
//    {
//        this->spec = spec;
//        sampleRate = spec.sampleRate;
//        // Prepare filters for the left channel
//        for (auto& filter : filters)
//        {
//            filter->prepare(spec);
//        }
//    }
//
//    void process (juce::dsp::AudioBlock<float>& block)
//    {
//        // Update filters if needed before processing
//        if (shouldUpdateFilters)
//        {
//            for (int i = 0; i < bands.size(); ++i)
//            {
//                Band band = bands[i];
//                if (i < filters.size())
//                    updateParametricBand (filters, i, sampleRate, band.freq, band.qFactor, band.ampl);
//            }
//            shouldUpdateFilters = false;
//        }
//
//        // Process channel
//        auto context = juce::dsp::ProcessContextReplacing (block);
//        for (auto& filter : filters)
//        {
//            filter->process (context);
//        }
//    }
//
//private:
//    std::vector<std::unique_ptr<Filter>> filters;
//    std::vector<Band> bands;
//    float sampleRate = 44100;
//    bool shouldUpdateFilters = false;
//    
//    juce::dsp::ProcessSpec spec;
//
//    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
//                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
//    {
//        auto coefficients = Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
//                                                         juce::Decibels::decibelsToGain (amplInDB));
//
//        addFilter (filters, coefficients);
//    }
//    
//    
//    void updateParametricBand (std::vector<std::unique_ptr<Filter>>& filters, int idx,
//                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
//    {
//        *filters[idx]->state = *Coefficients::makePeakFilter (sampleRate, centerFreq, qFactor,
//                                                              juce::Decibels::decibelsToGain (amplInDB));
//    }
//    
//    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
//                    const Coefficients::Ptr& coefficients)
//    {
//        auto filter = std::make_unique<Filter>();
//        *filter->state = *coefficients;
//        filter->prepare (spec);
//        filters.push_back (std::move(filter));
//    }
//};

#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <cmath>

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

/// This manages a list of filters for stereo processing, with separate filters for left and right channels.
class FilterChain
{
public:
    void setBands (const std::vector<Band>& bands, double sampleRate)
    {
        std::cout << "setting bands" << std::endl;
        this->bands = bands;
        this->sampleRate = sampleRate;
        this->shouldUpdateFilters = true;
        
        bool didAddBands = false;
        if (bands.size() > leftFilters.size())
        {
            for (int i = 0; i < bands.size(); ++i)
            {
                if (i >= leftFilters.size())
                {
                    Band band = bands[i];
                    addParametricBand (leftFilters, sampleRate, band.freq, band.qFactor, band.ampl);
                    addParametricBand (rightFilters, sampleRate, band.freq, band.qFactor, band.ampl);
                    didAddBands = true;
                }
            }
        }
        
        if (bands.size() < leftFilters.size())
        {
            leftFilters.erase (leftFilters.begin() + bands.size(), leftFilters.end());
            rightFilters.erase (rightFilters.begin() + bands.size(), rightFilters.end());
        }
        
        std::cout << "successfully set bands" << std::endl;
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        std::cout << "prepare filter chain" << std::endl;
        this->spec = spec;
        sampleRate = spec.sampleRate;
        // Prepare filters for the left channel
        for (auto& filter : leftFilters)
        {
            filter->prepare(spec);
        }
        for (auto& filter : rightFilters)
        {
            filter->prepare(spec);
        }
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        if (shouldUpdateFilters)
        {
            // Update filters if needed before processing
            for (int i = 0; i < bands.size(); ++i)
            {
                Band band = bands[i];
                if (i < leftFilters.size())
                {
                    updateParametricBand (leftFilters, i, sampleRate, band.freq, band.qFactor, band.ampl);
                    updateParametricBand (rightFilters, i, sampleRate, band.freq, band.qFactor, band.ampl);
                }
            }
            shouldUpdateFilters = false;
        }

        // Process left and right contexts
        auto leftBlock = block.getSingleChannelBlock (0);
        auto rightBlock = block.getSingleChannelBlock (1);

        juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);

        for (const auto& filter : leftFilters)
        {
            filter->process (leftContext);
        }
        
        for (const auto& filter : rightFilters)
        {
            filter->process (rightContext);
        }
    }

private:
    std::vector<std::unique_ptr<Filter>> leftFilters;
    std::vector<std::unique_ptr<Filter>> rightFilters;
    std::vector<Band> bands;
    float sampleRate = 44100;
    bool shouldUpdateFilters = false;
    
    juce::dsp::ProcessSpec spec;

    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        std::cout << "add parametric band in filter chain" << std::endl;
        auto filter = std::make_unique<Filter>();
        *filter->coefficients = *Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                              juce::Decibels::decibelsToGain (amplInDB));
        filter->prepare (spec);
        filters.push_back (std::move(filter));
    }
    
    
    void updateParametricBand (std::vector<std::unique_ptr<Filter>>& filters, int idx,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        std::cout << "update parametric band in filter chain" << std::endl;
        *filters[idx]->coefficients = *Coefficients::makePeakFilter (sampleRate, centerFreq, qFactor,
                                                              juce::Decibels::decibelsToGain (amplInDB));
    }
    
    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
                    const Coefficients::Ptr& coefficients)
    {
        
    }
};
