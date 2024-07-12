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
    std::cout << "start update" << std::endl;
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
    auto freqResponse = frequencyResponse (curve, numPoints);
    auto leftFreqResponse = freqResponse.first;
    auto rightFreqResponse = freqResponse.second;
    
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
    int numSamples = numPoints;
    
    juce::AudioBuffer<float> impulseBuffer (numChannels, numSamples);
    
    std::cout << "Copying from" << std::endl;
    
    std::cout << "Left Impulse Data: " << std::endl;
    std::cout << leftImpulseData[0] << std::endl;
//    for (int i = 0; i < numPoints; ++i)
//    {
//        std::cout << leftImpulseData[i] << " ";
//    }
//    std::cout << std::endl;
//    
//    std::cout << "Right Impulse Data: " << std::endl;
//    for (int i = 0; i < numPoints; ++i)
//    {
//        std::cout << rightImpulseData.at (i) << " ";
//    }
//    std::cout << std::endl;

    impulseBuffer.copyFrom(0, 0, leftImpulseData, numSamples);
    impulseBuffer.copyFrom(1, 0, rightImpulseData, numSamples);
    
    convolution.reset();
    convolution.loadImpulseResponse(std::move(impulseBuffer), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, juce::dsp::Convolution::Normalise::no);
    
    delete[] leftFreqResponse;
    delete[] rightFreqResponse;
}

std::pair<float*, float*> ArbitraryResponseFilter::frequencyResponse (const Curve& curve, int numPoints)
{
    float* leftFreqResponse = new float[2 * numPoints];
    float* rightFreqResponse = new float[2 * numPoints];
    for (int i = 0; i < 2 * numPoints; ++i)
    {
        float t = static_cast<float>(i) / (2 * numPoints);
        
        auto [val, val2] = curve.valueAtTime (t);
        
        if (i % 2 == 0)
        {
            leftFreqResponse[i] = val.real();
            rightFreqResponse[i] = val.real();
        }
        else
        {
            leftFreqResponse[i] = val.imag();
            rightFreqResponse[i] = val.imag();
        }
    }
    
    return { leftFreqResponse, rightFreqResponse };
}
