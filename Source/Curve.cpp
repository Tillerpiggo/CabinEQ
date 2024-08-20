/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const float Curve::compensatedValueAtFrequency (float frequency, float compensationSlope) const
{
    // Create amplitudes from eqNodes
    std::vector<float> values;
    for (const auto& curvePt : curvePts)
    {
        values.push_back (curvePt.val);
    }
    
    float valueAtFrequency = interpolateValueAtFrequency (frequency, values);
    float dbDifference = -compensationSlope * std::log2 ((frequency) / 1000.0f);
    valueAtFrequency += dbDifference;
    
    return juce::Decibels::decibelsToGain (valueAtFrequency);
}

const float Curve::valueAtFrequency (float frequency)
{
    if (cache.find(frequency) != cache.end())
    {
        return cache[frequency];
    }
    
    std::vector<float> values;
    for (const auto& curvePt : curvePts)
    {
        values.push_back (curvePt.val);
    }
    
    float valueAtFrequency = juce::Decibels::decibelsToGain (visualInterpolateAmplitudeAtFrequency(frequency));
    cache[frequency] = valueAtFrequency;
    
    return valueAtFrequency;
}

const float Curve::valueAtTime (float t)
{
    return compensatedValueAtFrequency (t * 22050, 0.0);
}

const std::vector<CurvePt>& Curve::getCurvePts()
{
    return curvePts;
}

float Curve::catmullRom(float t, float y0, float y1, float y2, float y3) const
{
    float t2 = t * t;
    float t3 = t * t * t;
    float y = 0.5f * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * t2 + (-y0 + 3.f * y1 - 3.f * y2 + y3) * t3);
    return y;
}

void Curve::updateWithCurvePts (std::vector<CurvePt> curvePts)
{
    std::sort(curvePts.begin(), curvePts.end(), [](const CurvePt &a, const CurvePt &b)
    {
        return a.freq < b.freq;
    });
    
    this->curvePts = curvePts;
    cache.clear();
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

const float Curve::interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const
{
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

const float Curve::visualInterpolateAmplitudeAtFrequency (const float frequency) const
{
    size_t numPoints = curvePts.size();
    
    auto logCompensation = [](float freq) { return 0.0f; };//-4.5 * std::log2(freq / 1000); };
    
    // Edge case checks
    if (curvePts.size() == 0) return logCompensation (frequency);
    if (frequency < curvePts.at(0).freq) return curvePts.at(0).val + logCompensation (frequency) - logCompensation (curvePts.at(0).freq);
    if (frequency > curvePts.at(numPoints - 1).freq) return curvePts.at(numPoints - 1).val + logCompensation (frequency) - logCompensation (curvePts.at(numPoints - 1).freq);
    
    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        float currFreq = curvePts.at(i).freq;
        if (frequency == currFreq)
        {
            return curvePts.at(i).val;
        }
        
        if (frequency < currFreq)
        {
            freq1 = curvePts.at(i - 1).freq;
            gain1 = curvePts.at(i - 1).val - logCompensation(freq1);
            freq2 = curvePts.at(i).freq;
            gain2 = curvePts.at(i).val - logCompensation(freq2);
            
            freq0 = (i > 1) ? curvePts.at(i - 2).freq : freq1 / 2;
            gain0 = (i > 1) ? curvePts.at(i - 2).val - logCompensation(freq0) : gain1;
            freq3 = (i < numPoints - 1) ? curvePts.at(i + 1).freq : freq2 * 2;
            gain3 = (i < numPoints - 1) ? curvePts.at(i + 1).val - logCompensation(freq3) : gain2;
            
            break;
        }
    }
    
    float logMinFreq = std::log(freq1);
    float logMaxFreq = std::log(freq2);
    float logFreq = std::log(frequency);

    // Normalize the log frequency
    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
    float gainAtFrequency = catmullRom(t, gain0, gain1, gain2, gain3);
    
    return gainAtFrequency + logCompensation (frequency);
}

/*
std::pair<float*, float*> Curve::frequencyResponse (int numPoints)
{
    float maxFreq = 60.0f;
    float minFreq = -48.0f;
    
    float* leftFreqResponse = new float[2 * numPoints];
    float* rightFreqResponse = new float[2 * numPoints];
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints);
        
        auto [val, val2] = valueAtTime (t);
        
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
        
        auto [val, val2] = valueAtTime (1 - t);
        
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

*/
