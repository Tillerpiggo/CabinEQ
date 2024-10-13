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

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

/// This manages a list of filters for stereo processing, with separate filters for left and right channels.
class FilterChain
{
public:
    void setBands (const std::vector<Band>& bands, double sampleRate)
    {
        filtersLeft.clear();
        filtersRight.clear();

        // Prepare filters for both left and right channels
        for (const auto& band : bands)
        {
            double Q = std::sqrt(std::pow(2.0, band.bandwidth)) / (std::pow(2.0, band.bandwidth) - 1);

            // Add the same parametric band to both channels
            addParametricBand(filtersLeft, sampleRate, band.freq, Q, band.ampl);
            addParametricBand(filtersRight, sampleRate, band.freq, Q, band.ampl);
        }
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        // Prepare filters for the left channel
        for (auto& filter : filtersLeft)
        {
            filter->prepare(spec);
        }

        // Prepare filters for the right channel
        for (auto& filter : filtersRight)
        {
            filter->prepare(spec);
        }
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        auto numChannels = block.getNumChannels();
        jassert(numChannels >= 2); // Ensure there are at least two channels for stereo

        // Get single channel blocks for left and right channels
        auto leftBlock = block.getSingleChannelBlock(0);
        auto rightBlock = block.getSingleChannelBlock(1);

        // Create processing contexts for each channel
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

        // Process left channel through its filter chain
        for (auto& filter : filtersLeft)
        {
            filter->process(leftContext);
        }

        // Process right channel through its filter chain
        for (auto& filter : filtersRight)
        {
            filter->process(rightContext);
        }
    }

private:
    std::vector<std::unique_ptr<Filter>> filtersLeft;
    std::vector<std::unique_ptr<Filter>> filtersRight;

    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        auto coefficients = Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                         juce::Decibels::decibelsToGain(amplInDB));

        addFilter(filters, coefficients);
    }
    
    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
                    const Coefficients::Ptr& coefficients)
    {
        auto filter = std::make_unique<Filter>();
        filter->coefficients = coefficients;
        filters.push_back(std::move(filter));
    }
};
