/*
  ==============================================================================

    ArbitraryResponseFilter.cpp
    Created: 14 Jun 2024 6:48:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitraryResponseFilter.h"

void ArbitraryResponseFilter::update (const Curve& curve, int fft_size)
{
//    std::vector<float> output(outputSize, 0.0f);
//
//    // Perform the convolution operation
//    for (int i = 0; i < outputSize; ++i) {
//        for (int j = 0; j < kernelSize; ++j) {
//            if (i - j >= 0 && i - j < signalSize) {
//                output[i] += signal[i - j] * kernel[j];
//            }
//        }
//    }
    
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
    std::cout << "desired response: " << std::endl;
    
    float* freqResponse = new float[2 * numPoints];
    for (int i = 0; i < 2 * numPoints; i++)
    {
        float t = static_cast<float>(i) / (2 * numPoints);
        if (i % 2 == 0)
        {
            freqResponse[i] = juce::Decibels::decibelsToGain (curve.valueAtTime(t).real());
            std::cout << freqResponse[i] << " ";
        }
        else
        {
            freqResponse[i] = curve.valueAtTime(t).imag();
        }
    }
    std::cout << std::endl;
    
    fft.performRealOnlyInverseTransform (freqResponse);
    
    juce::dsp::Convolution::Latency latency;
    latency.latencyInSamples = pow (2, 16);

    float* impulseData = freqResponse;
    
//    std::cout << "Impulse before: " << std::endl;
//    for (int i = 0; i < numPoints * 2; ++i)
//    {
//        std::cout << impulseData[i] << " ";
//    }
//    std::cout << std::endl;

    // Transform post-ringing into pre-ringing
    int quarterLength = numPoints / 2;

    // Swap elements of the first and second quarters
    for (int i = 0; i < quarterLength; ++i) 
    {
        std::swap(impulseData[i], impulseData[i + quarterLength]);
    }
    
//    std::cout << "Impulse after: " << std::endl;
//    for (int i = 0; i < numPoints * 2; ++i)
//    {
//        std::cout << impulseData[i] << " ";
//    }
//    std::cout << std::endl;
    
    // Window the filter
    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::flatTop, true);
    window.multiplyWithWindowingTable(impulseData, numPoints);
    
    std::cout << "Impulse after: " << std::endl;
    for (int i = 0; i < numPoints * 2; ++i)
    {
        std::cout << impulseData[i] << " ";
    }
    std::cout << std::endl;
    
    // Load the IR into the convolution
    int numChannels = 2;
    int numSamples = numPoints;
    
    juce::AudioBuffer<float> impulseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        impulseBuffer.copyFrom(ch, 0, impulseData, numSamples);
    }
    
    convolution.reset();
    convolution.loadImpulseResponse(std::move(impulseBuffer), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, juce::dsp::Convolution::Normalise::no);
    
    delete[] freqResponse;
}
