/*
  ==============================================================================

    BalanceArbitraryResponseFilter.cpp
    Created: 16 Jun 2024 9:35:44am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BalanceArbitraryResponseFilter.h"

void BalanceArbitraryResponseFilter::update()
{
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (8);
    int numPoints = fft.getSize();
    
    // Get both left and right frequency responses separately
    float* leftFreqResponse = new float[2 * numPoints];
    float* rightFreqResponse = new float[2 * numPoints];
    for (int i = 0; i < 2 * numPoints; i++)
    {
        float t = static_cast<float>(i) / (2 * numPoints);
        
        std::complex val = curve.valueAtTime (t);
        
        if (i % 2 == 0)
        {
            leftFreqResponse[i] = val.real();
            rightFreqResponse[i] = 24;//val.real();
        }
        else
        {
            leftFreqResponse[i] = val.imag();
            rightFreqResponse[i] = 0;//val.imag();
        }
        
        //std::cout << "val: " << val << std::endl;
    }
    
    fft.performRealOnlyInverseTransform (leftFreqResponse);
    fft.performRealOnlyInverseTransform (rightFreqResponse);

    float* leftImpulseData = leftFreqResponse;
    float* rightImpulseData = rightFreqResponse;
    
    // Transform post-ringing into pre-ringing
    int quarterLength = numPoints / 2;

    // Swap elements of the first and second quarters
    for (int i = 0; i < quarterLength; ++i) {
        std::swap(leftImpulseData[i], leftImpulseData[i + quarterLength]);
        std::swap(rightImpulseData[i], rightImpulseData[i + quarterLength]);
    }
    
    // Load the IR into the convolution
    int numChannels = 2;
    int numSamples = numPoints;
    double sampleRate = 44100;
    
    juce::AudioBuffer<float> impulseBuffer (numChannels, numSamples);
    
    impulseBuffer.copyFrom(0, 0, leftImpulseData, numSamples);
    impulseBuffer.copyFrom(1, 0, rightImpulseData, numSamples);
    
    juce::dsp::Convolution::Latency latency;
    latency.latencyInSamples = pow (2, 16);
    
    convolution.reset();
    convolution.loadImpulseResponse(std::move(impulseBuffer), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}
