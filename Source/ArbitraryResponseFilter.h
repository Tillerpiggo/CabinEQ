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
        : convolution (latency), latency { static_cast<int> (pow (2, 20)) } {}
    virtual ~ArbitraryResponseFilter() = default;
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        convolution.reset();
        sampleRate = spec.sampleRate;
        numChannels = spec.numChannels;
        std::cout << "prepare started 2" << std::endl;
        convolution.prepare (spec);
    }
    
    template <typename ProcessContext>
    void process (const ProcessContext &context) noexcept { convolution.process (context); }
    
    virtual std::pair<float*, float*> frequencyResponse (const Curve& curve, int numPoints);
    void update (const Curve& curve, int fft_size = 4); // update the filter to match the curve
    
protected:
    juce::dsp::Convolution convolution;
    juce::dsp::Convolution::Latency latency;
    
    double sampleRate = 44100;
    bool hasLoadedImpulse;
    
    int numChannels = 2;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArbitraryResponseFilter)
};
