/*
  ==============================================================================

    ArbitraryResponseFilter.cpp
    Created: 14 Jun 2024 6:48:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitraryResponseFilter.h"

void ArbitraryResponseFilter::updateWithCurve (Curve& curve, int fft_size)
{
    auto [leftImpulseData, rightImpulseData] = curve.getStereoImpulse (fft_size);
    
    // Load the IR into the convolution
    int numSamples = std::pow (2, fft_size);
    juce::AudioBuffer<float> impulseBuffer (numChannels, numSamples);

    impulseBuffer.copyFrom(0, 0, leftImpulseData, numSamples);
    impulseBuffer.copyFrom(1, 0, rightImpulseData, numSamples);
    
    convolution->reset();
    convolution->loadImpulseResponse(std::move(impulseBuffer), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, juce::dsp::Convolution::Normalise::no);
    
    delete[] leftImpulseData;
    delete[] rightImpulseData;
}
