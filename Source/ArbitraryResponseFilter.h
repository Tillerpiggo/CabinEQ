/*
  ==============================================================================

    ArbitraryResponseFilter.h
    Created: 14 Jun 2024 6:48:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class ArbitraryResponseFilter
{
public:
    ArbitraryResponseFilter ()
        : latency { static_cast<int> (pow (2, 18)) }
    {
        std::unique_ptr<juce::dsp::Convolution> newConvolver (new juce::dsp::Convolution (latency));
        convolution = std::move (newConvolver);
    }
    
    virtual ~ArbitraryResponseFilter() = default;
    
    template <typename ProcessContext>
    void process (const ProcessContext &context) noexcept { convolution->process (context); }
    void update (const Curve& curve, int fft_size = 4); // update the filter to match the curve
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        convolution->reset();
        sampleRate = spec.sampleRate;
        numChannels = spec.numChannels;
        std::cout << "prepare started 2" << std::endl;
        convolution->prepare (spec);
    }
    
protected:
    std::pair<float*, float*> frequencyResponse (const Curve& curve, int numPoints);
    
    std::unique_ptr<juce::dsp::Convolution> convolution;
    juce::dsp::Convolution::Latency latency;
    
    double sampleRate;
    int numChannels;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArbitraryResponseFilter)
};
