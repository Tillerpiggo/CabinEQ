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

//#pragma once
//
//#include <vector>
//#include <memory>
//#include <iostream>
//#include <cmath>
//#include <atomic>
//#include <juce_dsp/juce_dsp.h>
//
//using Filter = juce::dsp::IIR::Filter<float>;
//using Coefficients = juce::dsp::IIR::Coefficients<float>;
//
///// This manages a list of filters for stereo processing, with separate filters for left and right channels.
//class FilterChain
//{
//public:
//    void setBands (const std::vector<Band>& bands, double sampleRate)
//    {
//        // Build new filters in separate vectors
//        auto newFiltersLeft = std::make_shared<std::vector<std::unique_ptr<Filter>>>();
//        auto newFiltersRight = std::make_shared<std::vector<std::unique_ptr<Filter>>>();
//
//        for (const auto& band : bands)
//        {
//            double Q = std::sqrt(std::pow(2.0, band.bandwidth)) / (std::pow(2.0, band.bandwidth) - 1);
//
//            addParametricBand(*newFiltersLeft, sampleRate, band.freq, Q, band.ampl);
//            addParametricBand(*newFiltersRight, sampleRate, band.freq, Q, band.ampl);
//        }
//
//        // Prepare the new filters
//        juce::dsp::ProcessSpec spec;
//        spec.sampleRate = sampleRate;
//        spec.maximumBlockSize = 512; // adjust as needed
//        spec.numChannels = 1; // Since these are per-channel filters
//
//        for (auto& filter : *newFiltersLeft)
//        {
//            filter->prepare(spec);
//        }
//
//        for (auto& filter : *newFiltersRight)
//        {
//            filter->prepare(spec);
//        }
//
//        // Atomically swap the filters
//        filtersLeft.store(newFiltersLeft);
//        filtersRight.store(newFiltersRight);
//    }
//
//    void prepare (const juce::dsp::ProcessSpec& spec)
//    {
//        // Prepare filters if already set (optional)
//        auto currentFiltersLeft = filtersLeft.load();
//        auto currentFiltersRight = filtersRight.load();
//
//        if (currentFiltersLeft)
//        {
//            for (auto& filter : *currentFiltersLeft)
//                filter->prepare(spec);
//        }
//
//        if (currentFiltersRight)
//        {
//            for (auto& filter : *currentFiltersRight)
//                filter->prepare(spec);
//        }
//    }
//
//    void process (juce::dsp::AudioBlock<float>& block)
//    {
//        // Load the current filters atomically
//        auto currentFiltersLeft = filtersLeft.load();
//        auto currentFiltersRight = filtersRight.load();
//
//        auto numChannels = block.getNumChannels();
//        jassert(numChannels >= 2); // Ensure there are at least two channels for stereo
//
//        // Get single channel blocks for left and right channels
//        auto leftBlock = block.getSingleChannelBlock(0);
//        auto rightBlock = block.getSingleChannelBlock(1);
//
//        // Create processing contexts for each channel
//        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
//        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
//
//        // Process left channel through its filter chain
//        if (currentFiltersLeft)
//        {
//            for (auto& filter : *currentFiltersLeft)
//            {
//                filter->process(leftContext);
//            }
//        }
//
//        // Process right channel through its filter chain
//        if (currentFiltersRight)
//        {
//            for (auto& filter : *currentFiltersRight)
//            {
//                filter->process(rightContext);
//            }
//        }
//    }
//
//private:
//    std::atomic<std::shared_ptr<std::vector<std::unique_ptr<Filter>>>> filtersLeft { nullptr };
//    std::atomic<std::shared_ptr<std::vector<std::unique_ptr<Filter>>>> filtersRight { nullptr };
//
//    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
//                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
//    {
//        auto coefficients = Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
//                                                         juce::Decibels::decibelsToGain(amplInDB));
//        addFilter(filters, coefficients);
//    }
//
//    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
//                    const Coefficients::Ptr& coefficients)
//    {
//        auto filter = std::make_unique<Filter>();
//        filter->coefficients = coefficients;
//        filters.push_back(std::move(filter));
//    }
//};
