/*
  ==============================================================================

    ArbitraryResponseFilter.cpp
    Created: 14 Jun 2024 6:48:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitraryResponseFilter.h"

void ArbitraryResponseFilter::update (const Curve& curve)
{
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (4);
    int numPoints = fft.getSize();
    
    float* freqResponse = new float[2 * numPoints];
    for (int i = 0; i < 2 * numPoints; i++)
    {
        float t = static_cast<float>(i) / (2 * numPoints);
        if (i % 2 == 0)
        {
            freqResponse[i] = curve.valueAtTime(t).real();
        }
        else
        {
            freqResponse[i] = curve.valueAtTime(t).imag();
        }
    }
    
    fft.performRealOnlyInverseTransform (freqResponse);

    float* impulseData = freqResponse;

    // Transform post-ringing into pre-ringing
    int quarterLength = numPoints / 2;

    // Swap elements of the first and second quarters
    for (int i = 0; i < quarterLength; ++i) 
    {
        std::swap(impulseData[i], impulseData[i + quarterLength]);
    }
    
    // Load the IR into the convolution
    int numChannels = 2;
    int numSamples = numPoints;
    
    juce::AudioBuffer<float> impulseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        impulseBuffer.copyFrom(ch, 0, impulseData, numSamples);
    }
    
    juce::dsp::Convolution::Latency latency;
    latency.latencyInSamples = pow (2, 16);
    
    convolution.reset();
    convolution.loadImpulseResponse(std::move(impulseBuffer), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}
