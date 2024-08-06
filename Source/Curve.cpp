/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const std::pair<std::complex<float>, std::complex<float>> Curve::compensatedValueAtFrequency (float frequency, float compensationSlope) const
{
    std::vector<float> amplitudes;
    std::vector<float> pans;
    
    for (const auto& eqNode : eqNodes)
    {
        amplitudes.push_back (eqNode.amplitude);
        pans.push_back (eqNode.pan);
    }
    
    float amplitudeAtFrequency = interpolateValueAtFrequency (frequency, amplitudes);
    float panAtFrequency = interpolateValueAtFrequency (frequency, pans);
    
//    // Apply compensationSlope of "tilt" - db/oct
//    float dbDifference = compensationSlope * std::log2((frequency) / 1000.0f);
    
    // Apply compensation of average
    float dbDifference = avgValueAtFrequency (frequency, 0.0f).first.real();
    float leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    float rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;

    float leftGain = juce::Decibels::decibelsToGain (leftDB);
    float rightGain = juce::Decibels::decibelsToGain (rightDB);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, 0.0f);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::avgValueAtFrequency (float frequency, float compensationSlope) const
{
    float freqStep = 1.05;
    float avg = 0.0f;
    int numFreqs = 20;
    for (int i = -numFreqs / 2; i <= numFreqs / 2; ++i)
    {
        avg += utilValueAtFrequency (frequency * std::pow (freqStep, i), compensationSlope).first.real();
    }
    avg /= static_cast<float> (numFreqs);
    
    return { std::complex<float> (avg), std::complex<float> (avg) };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::utilValueAtFrequency (float frequency, float compensationSlope) const
{
    std::vector<float> amplitudes;
    std::vector<float> pans;
    
    for (const auto& eqNode : eqNodes)
    {
        amplitudes.push_back (eqNode.amplitude);
        pans.push_back (eqNode.pan);
    }
    
    float amplitudeAtFrequency = interpolateValueAtFrequency (frequency, amplitudes);
    float panAtFrequency = interpolateValueAtFrequency (frequency, pans);
    
    // Apply compensationSlope of "tilt" - db/oct
    float dbDifference = compensationSlope * std::log2((frequency) / 1000.0f);
    float leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    float rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;

    float leftGain = juce::Decibels::decibelsToGain (leftDB);
    float rightGain = juce::Decibels::decibelsToGain (rightDB);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, 0.0f);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtFrequency (float frequency)
{
    if (cache.find(frequency) != cache.end())
    {
        return cache[frequency];
    }
    
    std::vector<float> amplitudes;
    std::vector<float> pans;
    
    for (const auto& eqNode : eqNodes)
    {
        amplitudes.push_back (eqNode.amplitude);
        pans.push_back (eqNode.pan);
    }
    
    float amplitudeAtFrequency = visualInterpolateAmplitudeAtFrequency(frequency);
    float panAtFrequency = 0.0f;//interpolateValueAtFrequency (frequency, pans);
    
    // don't apply any extra compensation
    float leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency;
    float rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency;
    
//    // Make it render as flat
//    if (eqNodes.size() > 0)
//    {
//        float minNodeFreq = eqNodes[0].frequency;
//        float maxNodeFreq = eqNodes[eqNodes.size() - 1].frequency;
//        float slope = -4.5;
//        float dbDifference = 0;
//        if (frequency < eqNodes[0].frequency)
//            dbDifference = slope * std::log2 (frequency / minNodeFreq);
//        if (frequency > eqNodes[eqNodes.size() - 1].frequency)
//            dbDifference = slope * std::log2 (frequency / maxNodeFreq);
//        
//        leftDB += dbDifference;
//        rightDB += dbDifference;
//    }

    float leftGain = juce::Decibels::decibelsToGain (leftDB);
    float rightGain = juce::Decibels::decibelsToGain (rightDB);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, 0.0f);
    
    cache[frequency] = { leftVal, rightVal };
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtTime (float t)
{
    return compensatedValueAtFrequency (t * 22050, -3.55);
}

const std::pair<std::complex<float>, std::complex<float>> Curve::scaleComplexPair (std::pair<std::complex<float>, std::complex<float>> pair, float scalar) const
{
    return { pair.first * scalar, pair.second * scalar };
}

const std::vector<EQNode>& Curve::getEQNodes()
{
    return eqNodes;
}

float Curve::catmullRom(float t, float y0, float y1, float y2, float y3) const
{
    float t2 = t * t;
    float t3 = t * t * t;
    float y = 0.5f * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * t2 + (-y0 + 3.f * y1 - 3.f * y2 + y3) * t3);
    return y;
}

/*
using Point = std::array<float, 2>;
Point Curve::catmullRom(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t) {
    double t2 = t * t;
    double t3 = t2 * t;

    Point result;

    result[0] = 0.5 * (
        (2 * P1[0]) +
        (-P0[0] + P2[0]) * t +
        (2 * P0[0] - 5 * P1[0] + 4 * P2[0] - P3[0]) * t2 +
        (-P0[0] + 3 * P1[0] - 3 * P2[0] + P3[0]) * t3
    );

    result[1] = 0.5 * (
        (2 * P1[1]) +
        (-P0[1] + P2[1]) * t +
        (2 * P0[1] - 5 * P1[1] + 4 * P2[1] - P3[1]) * t2 +
        (-P0[1] + 3 * P1[1] - 3 * P2[1] + P3[1]) * t3
    );

    return result;
}
 */

void Curve::updateWithEQNodes (std::vector<EQNode> eqNodes)
{
    std::sort(eqNodes.begin(), eqNodes.end(), [](const EQNode &a, const EQNode &b) 
    {
        return a.frequency < b.frequency;
    });
    
    this->eqNodes = eqNodes;
    cache.clear();
}

const std::pair<float*, float*> Curve::getStereoImpulse (int fft_size)
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

const float Curve::interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const
{
    size_t numPoints = eqNodes.size();
    
    // Edge case checks
    if (eqNodes.size() == 0) return 0.0f;
    if (frequency < eqNodes.at (0).frequency) return values.at (0);
    if (frequency > eqNodes.at(numPoints - 1).frequency) return values.at (numPoints - 1);
    
    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        float currFreq = eqNodes.at (i).frequency;
        if (frequency == currFreq)
        {
            return values.at(i);
        }
        
        if (frequency < currFreq)
        {
            freq1 = eqNodes.at(i - 1).frequency;
            gain1 = values.at(i - 1);
            freq2 = eqNodes.at(i).frequency;
            gain2 = values.at(i);
            
            gain0 = (i > 1) ? values.at (i - 2) : gain1;
            freq0 = (i > 1) ? eqNodes.at (i - 2).frequency : freq1;
            gain3 = (i < numPoints - 1) ? values.at (i + 1) : gain2;
            freq3 = (i < numPoints - 1) ? eqNodes.at (i + 1).frequency : freq2;
            
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
    size_t numPoints = eqNodes.size();
    
    auto logCompensation = [](float freq) { return -4.5 * std::log2(freq / 1000); };
    
    // Edge case checks
    if (eqNodes.size() == 0) return logCompensation (frequency);
    if (frequency < eqNodes.at(0).frequency) return eqNodes.at(0).amplitude + logCompensation (frequency) - logCompensation (eqNodes.at(0).frequency);
    if (frequency > eqNodes.at(numPoints - 1).frequency) return eqNodes.at(numPoints - 1).amplitude + logCompensation (frequency) - logCompensation (eqNodes.at(numPoints - 1).frequency);
    
    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        float currFreq = eqNodes.at(i).frequency;
        if (frequency == currFreq)
        {
            return eqNodes.at(i).amplitude;
        }
        
        if (frequency < currFreq)
        {
            freq1 = eqNodes.at(i - 1).frequency;
            gain1 = eqNodes.at(i - 1).amplitude - logCompensation(freq1);
            freq2 = eqNodes.at(i).frequency;
            gain2 = eqNodes.at(i).amplitude - logCompensation(freq2);
            
            freq0 = (i > 1) ? eqNodes.at(i - 2).frequency : freq1 / 2;
            gain0 = (i > 1) ? eqNodes.at(i - 2).amplitude - logCompensation(freq0) : gain1;
            freq3 = (i < numPoints - 1) ? eqNodes.at(i + 1).frequency : freq2 * 2;
            gain3 = (i < numPoints - 1) ? eqNodes.at(i + 1).amplitude - logCompensation(freq3) : gain2;
            
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
