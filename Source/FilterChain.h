/*
  ==============================================================================

    FilterChain.h
    Created: 13 Oct 2024 1:06:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <cmath>

#include "SIMDIIRFilter.h"

using Filter = juce::dsp::IIR::Filter<float>;
using Coefficients = juce::dsp::IIR::Coefficients<float>;

/// This manages a list of filters for stereo processing, with separate filters for left and right channels.
class FilterChain
{
public:
    void setBands (const std::vector<Band>& bands, double sampleRate)
    {
        this->bands = bands;
        this->sampleRate = sampleRate;
        this->shouldUpdateFilters = true;
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
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
            // Add necessary filters
            if (bands.size() > leftFilters.size())
            {
                for (int i = 0; i < bands.size(); ++i)
                {
                    if (i >= leftFilters.size())
                    {
                        Band band = bands[i];
                        addParametricBand (leftFilters, sampleRate, band.freq, band.qFactor, band.type != Band::Type::right ? band.ampl : 0);
                        addParametricBand (rightFilters, sampleRate, band.freq, band.qFactor, band.type != Band::Type::left ? band.ampl : 0);
                    }
                }
            }
            
            // Remove unnecessary filters
            if (bands.size() < leftFilters.size())
            {
                leftFilters.erase (leftFilters.begin() + bands.size(), leftFilters.end());
                rightFilters.erase (rightFilters.begin() + bands.size(), rightFilters.end());
            }
            
            // Update filters if needed before processing
            for (int i = 0; i < bands.size(); ++i)
            {
                Band band = bands[i];
                if (i < leftFilters.size())
                {
                    float shuffleFactor = 1.0f - shuffle * 0.1f;
                    if (i % 2 == 0)
                        shuffleFactor = 1.0f - shuffle * 0.7f;
                    updateParametricBand (leftFilters, i, sampleRate, band.freq * pitch * shuffleFactor, band.qFactor, band.type != Band::Type::right ? band.ampl : 0);
                    updateParametricBand (rightFilters, i, sampleRate, band.freq * pitch * shuffleFactor, band.qFactor, band.type != Band::Type::left ? band.ampl : 0);
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
    
    void setPitch (float pitch)
    {
        this->pitch = pitch;
        shouldUpdateFilters = true;
    }
    
    void setShuffle (float shuffle)
    {
        this->shuffle = shuffle;
        shouldUpdateFilters = true;
    }

private:
    std::vector<std::unique_ptr<Filter>> leftFilters;
    std::vector<std::unique_ptr<Filter>> rightFilters;
//    std::vector<std::unique_ptr<SIMDIIRFilter>> leftFilters;
//    std::vector<std::unique_ptr<SIMDIIRFilter>> rightFilters;
    std::vector<Band> bands;
    float sampleRate = 44100;
    bool shouldUpdateFilters = false;
    
    juce::dsp::ProcessSpec spec;
    
    float pitch = 1.0f;
    float shuffle = 0.0f;

    void addParametricBand (std::vector<std::unique_ptr<Filter>>& filters,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        auto filter = std::make_unique<Filter>();
        *filter->coefficients = *Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                              juce::Decibels::decibelsToGain (amplInDB));
//        filter->setCoefficients (sampleRate, centerFreq, qFactor, amplInDB);
        filter->prepare (spec);
        filters.push_back (std::move(filter));
    }
    
    
    void updateParametricBand (std::vector<std::unique_ptr<Filter>>& filters, int idx,
                            double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        *filters[idx]->coefficients = *Coefficients::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                                    juce::Decibels::decibelsToGain (amplInDB));
//        filters[idx]->setCoefficients (sampleRate, centerFreq, qFactor, amplInDB);
//        *filters[idx]->coefficients = *Coefficients::makeNotch (sampleRate, centerFreq, qFactor);
    }
    
//    void addFilter (std::vector<std::unique_ptr<Filter>>& filters,
//                    const Coefficients::Ptr& coefficients)
//    {
//        
//    }
};
