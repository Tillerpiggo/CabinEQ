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
    /*
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
    
    for (int i = 0; i < numPoints * 2; ++i)
    {
        leftImpulseData[i] = leftFreqResponse[i];
        rightImpulseData[i] = rightFreqResponse[i];
    }

    // Transform post-ringing into pre-ringing
    int inflectionPoint = numPoints / 2;
    
    for (int i = 0; i < numPoints / 2; ++i)
    {
        std::swap(leftImpulseData[i], leftImpulseData[i + numPoints / 2]);
        std::swap(rightImpulseData[i], rightImpulseData[i + numPoints / 2]);
    }
    
    // Window the impulse
    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::blackmanHarris, true);
    window.multiplyWithWindowingTable(leftImpulseData, numPoints);
    window.multiplyWithWindowingTable(rightImpulseData, numPoints);
    */
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

std::pair<float*, float*> ArbitraryResponseFilter::frequencyResponse (const Curve& curve, int numPoints)
{
    //float factor = nyquist / maxFreq;
    
    float maxFreq = 60.0f;
    float minFreq = -48.0f;
    
    float* leftFreqResponse = new float[2 * numPoints];
    float* rightFreqResponse = new float[2 * numPoints];
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints);
        
        auto [val, val2] = curve.valueAtTime (t);
        
        if (i % 2 == 0)
        {
            leftFreqResponse[i] = val.real();
            rightFreqResponse[i] = val2.real();
            
            // Limit freq response
            if (leftFreqResponse[i] > maxFreq) leftFreqResponse[i] = maxFreq;
            if (leftFreqResponse[i] < minFreq) leftFreqResponse[i] = minFreq;
            if (rightFreqResponse[i] > maxFreq) rightFreqResponse[i] = maxFreq;
            if (rightFreqResponse[i] < minFreq) rightFreqResponse[i] = minFreq;
        }
        else
        {
            leftFreqResponse[i] = val.imag();
            rightFreqResponse[i] = val2.imag();
        }
    }
    
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints);
        
        auto [val, val2] = curve.valueAtTime (1 - t);
        
        if (i % 2 == 0)
        {
            leftFreqResponse[i + numPoints] = val.real();
            rightFreqResponse[i + numPoints] = val2.real();
            
            // Limit freq response
            if (leftFreqResponse[i + numPoints] > maxFreq) leftFreqResponse[i + numPoints] = maxFreq;
            if (leftFreqResponse[i + numPoints] < minFreq) leftFreqResponse[i + numPoints] = minFreq;
            if (rightFreqResponse[i + numPoints] > maxFreq) rightFreqResponse[i + numPoints] = maxFreq;
            if (rightFreqResponse[i + numPoints] < minFreq) rightFreqResponse[i + numPoints] = minFreq;
        }
        else
        {
            leftFreqResponse[i + numPoints] = val.imag();
            rightFreqResponse[i + numPoints] = val2.imag();
        }
    }
    
    return { leftFreqResponse, rightFreqResponse };
}
