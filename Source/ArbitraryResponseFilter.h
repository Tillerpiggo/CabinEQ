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
#include "ImpulseResponseLoaderThread.h"

class ArbitraryResponseFilter
{
public:
    ArbitraryResponseFilter (int fftSize, int delayInSamples = 0)
        : latency { static_cast<int> (pow (2, fftSize / 2)) + delayInSamples }
    {
        std::unique_ptr<juce::dsp::Convolution> newConvolver (new juce::dsp::Convolution (latency));
        convolution = std::move (newConvolver);
    }
    
    virtual ~ArbitraryResponseFilter() = default;
    
    void process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept { convolution->process (context); }
    void updateWithCurve (Curve& amplCurve, int fft_size = 14);
    void generateAndLoadImpulseResponse(Curve& amplCurve, int fft_size);
    
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        convolution->reset();
        sampleRate = spec.sampleRate;
        numChannels = spec.numChannels;
        convolution->prepare (spec);
    }
    
protected:
    std::unique_ptr<juce::dsp::Convolution> convolution;
    juce::dsp::Convolution::Latency latency;
    
    double sampleRate;
    int numChannels;
    
    // Multi-threading
    std::unique_ptr<ImpulseResponseLoaderThread> impulseResponseLoaderThread;
    juce::CriticalSection convolutionLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArbitraryResponseFilter)
};
