/*
  ==============================================================================

    ArbitraryResponseFilter.cpp
    Created: 14 Jun 2024 6:48:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArbitraryResponseFilter.h"

void ArbitraryResponseFilter::updateWithCurves (Curve& leftAmplCurve, Curve& rightAmplCurve, int fft_size)
{
    // Get left & right impulse data
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
//    auto amplResponse = amplCurve.getFrequencyResponse (numPoints);
//    auto panResponse = panCurve.getFrequencyResponse (numPoints);
    
    auto [leftFreqResp, rightFreqResp] = Curve::getStereoFrequencyResponse (leftAmplCurve, rightAmplCurve, numPoints);
    
    float* leftFreqResponse = new float[2 * numPoints];
    float* rightFreqResponse = new float[2 * numPoints];
    
    for (int i = 0; i < 2 * numPoints; ++i)
    {
        if (i % 2 == 0)
        {
            leftFreqResponse[i] = leftFreqResp[i];
            rightFreqResponse[i] = rightFreqResp[i];
        }
        else
        {
            leftFreqResponse[i] = 0;
            rightFreqResponse[i] = 0;
        }
//        leftFreqResponse[i] = leftFreqResp[i];
//        rightFreqResponse[i] = rightFreqResp[i];
    }
    
    fft.performRealOnlyInverseTransform (leftFreqResponse);
    fft.performRealOnlyInverseTransform (rightFreqResponse);
    
    float* leftImpulseData = leftFreqResponse;
    float* rightImpulseData = rightFreqResponse;
    
    // idk if this is necessary or if it even does anything
    for (int i = 0; i < numPoints * 2; ++i)
    {
        leftImpulseData[i] = leftFreqResponse[i];
        rightImpulseData[i] = rightFreqResponse[i];
    }

    // Transform post-ringing into pre-ringing
    for (int i = 0; i < numPoints / 2; ++i)
    {
        std::swap(leftImpulseData[i], leftImpulseData[i + numPoints / 2]);
        std::swap(rightImpulseData[i], rightImpulseData[i + numPoints / 2]);
    }
    
    // Window the impulse
    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::rectangular, true);
    window.multiplyWithWindowingTable(leftImpulseData, numPoints);
    window.multiplyWithWindowingTable(rightImpulseData, numPoints);
    
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
