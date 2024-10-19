/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const float Curve::valueAtTime (float t)
{
    float freq = t * 22050;
    float dbOffset = -4.5f * std::log2 (std::max (freq, 20.0f) / 1000.0f);
    return dbOffset;
}
/*
const float* Curve::getImpulse (int fft_size)
{
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
    auto freqResponse = frequencyResponse (numPoints);
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
    for (int i = 0; i < numPoints / 2; ++i)
    {
        std::swap(leftImpulseData[i], leftImpulseData[i + numPoints / 2]);
        std::swap(rightImpulseData[i], rightImpulseData[i + numPoints / 2]);
    }
    
    // Window the impulse
    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::rectangular, true);
    window.multiplyWithWindowingTable(leftImpulseData, numPoints);
    window.multiplyWithWindowingTable(rightImpulseData, numPoints);
    
    return { leftImpulseData, rightImpulseData };
}
 */
std::vector<float> Curve::getFrequencyResponse (int numPoints)
{
    float maxFreq = 60.0f;
    float minFreq = -48.0f;
    
    std::vector<float> freqResponse (2 * numPoints, 0);
    
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints);
        
        auto val = juce::Decibels::decibelsToGain (valueAtTime (t));
        
        if (i % 2 == 0)
        {
            if (val > maxFreq) val = maxFreq;
            if (val < minFreq) val = minFreq;
            freqResponse[i] = val;
        }
        else
        {
            freqResponse[i] = 0;
        }
    }
    
    for (int i = 0; i < numPoints / 2; ++i)
    {
        freqResponse[2 * i + numPoints] = freqResponse[numPoints - 2 * (i + 1)];
        freqResponse[2 * i + numPoints + 1] = freqResponse[numPoints - 2 * (i + 1) + 1];
    }
    
    return freqResponse;
}

std::pair<std::vector<float>, std::vector<float>> Curve::getStereoFrequencyResponse (Curve& amplCurve, int numPoints)
{
    std::vector<float> leftFreqResponse (2 * numPoints, 0.0f);
    std::vector<float> rightFreqResponse (2 * numPoints, 0.0f);

    // Compute the positive frequencies (including DC and Nyquist)
    for (int i = 0; i <= numPoints / 2; ++i)
    {
        float t = static_cast<float>(i) / (numPoints / 2);

        float ampl = amplCurve.valueAtTime (t);
        float gain = juce::Decibels::decibelsToGain (ampl);

        leftFreqResponse[2 * i] = gain;
        leftFreqResponse[2 * i + 1] = 0;

        rightFreqResponse[2 * i] = gain;
        rightFreqResponse[2 * i + 1] = 0;
    }

    // Compute the negative frequencies by ensuring conjugate symmetry
    for (int i = 1; i < numPoints / 2; ++i)
    {
        int reverseIdx = numPoints - i;

        // Conjugate symmetry for the left channel
        leftFreqResponse[2 * reverseIdx] = leftFreqResponse[2 * i];
        leftFreqResponse[2 * reverseIdx + 1] = -leftFreqResponse[2 * i + 1];

        // Conjugate symmetry for the right channel
        rightFreqResponse[2 * reverseIdx] = rightFreqResponse[2 * i];
        rightFreqResponse[2 * reverseIdx + 1] = -rightFreqResponse[2 * i + 1];
    }

    return { leftFreqResponse, rightFreqResponse };
}
