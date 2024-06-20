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
    ArbitraryResponseFilter (const Curve& curve)
    : curve (curve), convolution (latency), latency { static_cast<int>(pow (2, 12))} {}
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        convolution.reset();
        sampleRate = spec.sampleRate;
        convolution.prepare (spec);
    }
    template <typename ProcessContext>
    void process (const ProcessContext &context) noexcept 
    {
        convolution.process (context);
        
//        if (!hasLoadedImpulse)
//        {
//            juce::File file ("/Users/tylergee/Desktop/Coding/StartupMVP/Resources/cathedral.wav");
//            convolution.loadImpulseResponse (file,
//                                             juce::dsp::Convolution::Stereo::yes,
//                                             juce::dsp::Convolution::Trim::no,
//                                             0);
//            hasLoadedImpulse = true;
//        }
    };
    
    virtual void update(); // update the filter to match the curve
    
protected:
    const Curve& curve;
    juce::dsp::Convolution convolution;
    juce::dsp::Convolution::Latency latency;
    double sampleRate = 44100;
    bool hasLoadedImpulse;
    
};
