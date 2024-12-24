

/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const std::pair<float, float> Curve::valueAtFrequency (float frequency) const
{
    float leftValueAtFrequency = interpolateValueAtFrequency (frequency, leftCurvePts);
    float rightValueAtFrequency = interpolateValueAtFrequency (frequency, rightCurvePts);
    return { leftValueAtFrequency, rightValueAtFrequency };
}

const std::pair<float, float> Curve::valueAtTime (float t)
{
    return valueAtFrequency (t * 22050);
}

float Curve::catmullRom(float t, float y0, float y1, float y2, float y3) const
{
    float t2 = t * t;
    float t3 = t * t * t;
    float y = 0.5f * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * t2 + (-y0 + 3.f * y1 - 3.f * y2 + y3) * t3);
    return y;
}

void Curve::updateWithCurvePts (std::vector<CurvePt> leftCurvePts, std::vector<CurvePt> rightCurvePts)
{
    std::sort(leftCurvePts.begin(), leftCurvePts.end(), [](const CurvePt &a, const CurvePt &b)
    {
        return a.freq < b.freq;
    });
    
    std::sort(rightCurvePts.begin(), rightCurvePts.end(), [](const CurvePt &a, const CurvePt &b)
    {
        return a.freq < b.freq;
    });
    
    this->leftCurvePts = leftCurvePts;
    this->rightCurvePts = rightCurvePts;
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

const float Curve::interpolateValueAtFrequency (const float frequency, const std::vector<CurvePt>& curvePts) const
{
    // Create amplitudes from eqNodes
    std::vector<float> values;
    for (const auto& curvePt : curvePts)
    {
        values.push_back (curvePt.val);
    }
    
    size_t numPoints = curvePts.size();
    
    // Edge case checks
    if (curvePts.size() == 0) return 0.0f;
    if (frequency < curvePts.at (0).freq) return values.at (0);
    if (frequency > curvePts.at(numPoints - 1).freq) return values.at (numPoints - 1);
    
    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        float currFreq = curvePts.at (i).freq;
        if (frequency == currFreq)
        {
            return values.at(i);
        }
        
        if (frequency < currFreq)
        {
            freq1 = curvePts.at(i - 1).freq;
            gain1 = values.at(i - 1);
            freq2 = curvePts.at(i).freq;
            gain2 = values.at(i);
            
            gain0 = (i > 1) ? values.at (i - 2) : gain1;
            freq0 = (i > 1) ? curvePts.at (i - 2).freq : freq1;
            gain3 = (i < numPoints - 1) ? values.at (i + 1) : gain2;
            freq3 = (i < numPoints - 1) ? curvePts.at (i + 1).freq : freq2;
            
            break;
        }
    }
    
    float logMinFreq = std::log(freq1);
    float logMaxFreq = std::log(freq2);
    float logFreq = std::log(frequency);

    // Normalize the log frequency
    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
    float gainAtFrequency = catmullRom (t, gain0, gain1, gain2, gain3);
    
    return gainAtFrequency;
}

std::pair<std::vector<float>, std::vector<float>> Curve::getStereoFrequencyResponse (Curve& amplCurve, int numPoints)
{
    std::vector<float> leftFreqResponse (2 * numPoints, 0.0f);
    std::vector<float> rightFreqResponse (2 * numPoints, 0.0f);

    // Compute the positive frequencies (including DC and Nyquist)
    for (int i = 0; i <= numPoints / 2; ++i)
    {
        float t = static_cast<float>(i) / (numPoints / 2);

        auto [leftDb, rightDb] = amplCurve.valueAtTime (t);
        float leftGain = juce::Decibels::decibelsToGain (leftDb);
        float rightGain = juce::Decibels::decibelsToGain (rightDb);

        leftFreqResponse[2 * i] = leftGain;
        leftFreqResponse[2 * i + 1] = 0;

        rightFreqResponse[2 * i] = rightGain;
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
