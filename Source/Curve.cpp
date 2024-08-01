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
    
    float dbDifference = compensationSlope * std::log2((frequency) / 1000.0f); // commented out for CabinEQ dev purposes
//    float dbDifference = 0.0f;
//    float dbDifference = -3.5f * std::log2((frequency) / 1000.0f);
//    float dbDifference = -inverseFM.valueAtFrequency (frequency, 82.5);
//    if (frequency < 100.0)
//        dbDifference = 0;
    
    float leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    float rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    
//    if (leftDB < 0 || rightDB < 0 || 1)
//    {
//        amplitudeAtFrequency = interpolateValueAtFrequency (0.4079 * frequency + 10, amplitudes);
//        panAtFrequency = interpolateValueAtFrequency (0.4079 * frequency + 10, pans);
//        leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
//        rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
//        
//        float factor = std::abs(leftDB) / 10.0;
//        leftDB *= factor;
//        rightDB *= factor;
//    }

    float leftGain = juce::Decibels::decibelsToGain (leftDB);
    float rightGain = juce::Decibels::decibelsToGain (rightDB);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, 0.0f);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtFrequency (float frequency) const
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
    
    /*float dbDifference = -4.5f * std::log2((frequency) / 1000.0f);*/ // commented out for CabinEQ dev purposes
    float dbDifference = 0.0f;
//    float dbDifference = -3.0f * std::log2((frequency) / 1000.0f);
//    float dbDifference = 0.0f;
    
    float leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    float rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
    
//    if (1)//leftDB < 0 || rightDB < 0)
//    {
//        amplitudeAtFrequency = interpolateValueAtFrequency (0.4079 * frequency + 10, amplitudes);
//        panAtFrequency = interpolateValueAtFrequency (0.4079 * frequency + 10, pans);
//        leftDB = -0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
//        rightDB = 0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference;
//        
//        float factor = 1.0f;//std::abs(leftDB) / 20.0;
//        leftDB *= factor;
//        rightDB *= factor;
//    }

    float leftGain = juce::Decibels::decibelsToGain (leftDB);
    float rightGain = juce::Decibels::decibelsToGain (rightDB);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, 0.0f);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtTime (float t) const
{
    
//    float scaleFactor1 = 0.7;
//    if (t * 22050 > 5000)
//        scaleFactor1 *= (22050 - (t * 22050)) / 5000;
    float timeFactor = 1.0;
    
    if (t < 0.2)
        timeFactor -= 3 * std::abs (t - 0.2);
    
    float slopeFactor = 0.4;
    if (t > 0.2)
        slopeFactor += 3.0 * (t - 0.2);
    
    auto undertones = scaleComplexPair (compensatedValueAtFrequency(timeFactor * t * 17990 + 10, -5.5 + 3 * slopeFactor), 1.0);
//    auto undertones = scaleComplexPair (compensatedValueAtFrequency (0.5 * t * 22050, -4.5), 1.0);
    
    float scaleFactor2 = 1.0;
    float cutoff = 200;
    if (t * 22050 < cutoff)
        scaleFactor2 *= (t * 22050) / cutoff;
    auto normaltones = scaleComplexPair (compensatedValueAtFrequency (t * 22050, -2.5), scaleFactor2);
    auto overtones = scaleComplexPair (compensatedValueAtFrequency (t * 2.457 * 22050 - 10, 0.0), 1.0);
    auto res = multiplyComplexPair (undertones, overtones);
    res = scaleComplexPair (res, 0.5);
    
//    undertones = reciprocalComplexPair (undertones);
    
//    auto undertones = scaleComplexPair (compensatedValueAtFrequency ((0.5 * t) * 17990 + 10, -4.5), 1);
    return undertones;
}

const std::pair<std::complex<float>, std::complex<float>> Curve::addComplexPair (std::pair<std::complex<float>, std::complex<float>> pair1, std::pair<std::complex<float>, std::complex<float>> pair2) const
{
    return { pair1.first + pair2.first, pair1.second + pair2.second };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::multiplyComplexPair (std::pair<std::complex<float>, std::complex<float>> pair1, std::pair<std::complex<float>, std::complex<float>> pair2) const
{
    return { pair1.first * pair2.first, pair1.second * pair2.second };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::reciprocalComplexPair (std::pair<std::complex<float>, std::complex<float>> pair) const
{
    return { std::complex<float> (1) / pair.first, std::complex<float> (1) / pair.second };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::scaleComplexPair (std::pair<std::complex<float>, std::complex<float>> pair, float scalar) const
{
    return { pair.first * scalar, pair.second * scalar };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtNormalizedTime (float t) const
{
    float minFreq = 20;//frequencies.at(0);
    float maxFreq = 22050;//frequencies.at(frequencies.size() - 1);
    
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(minFreq);
    float logMaxFreq = std::log(maxFreq);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    auto undertones = scaleComplexPair (compensatedValueAtFrequency(freq / 2.0, -4.5), 1);
    auto normaltones = scaleComplexPair (compensatedValueAtFrequency (freq, -3.5), 1);
    auto res = addComplexPair (undertones, normaltones);
    
    undertones = scaleComplexPair (compensatedValueAtFrequency ((0.5 * t) * 22050 + 0, -3.8), 1);
    
    return undertones;
}

float Curve::catmullRom(float t, float y0, float y1, float y2, float y3) const
{
    float t2 = t * t;
    float t3 = t * t * t;
    float y = 0.5f * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * t2 + (-y0 + 3.f * y1 - 3.f * y2 + y3) * t3);
    return y;
}

void Curve::updateWithEQNodes (std::vector<EQNode> eqNodes)
{
    std::sort(eqNodes.begin(), eqNodes.end(), [](const EQNode &a, const EQNode &b) 
    {
        return a.frequency < b.frequency;
    });
    
    this->eqNodes = eqNodes;
}

const std::pair<float*, float*> Curve::getStereoImpulse (int fft_size) const
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
    int inflectionPoint = numPoints / 2;
    
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
    
    float freq1, freq2;
    float gain0, gain1, gain2, gain3;
    
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
            freq2 = eqNodes.at(i).frequency;
            gain1 = values.at(i - 1);
            gain2 = values.at(i);
            
            gain0 = (i > 1) ? values.at(i - 2) : gain1;
            gain3 = (i < numPoints - 1) ? values.at(i + 1) : gain2;
            
            break;
        }
    }
    
    float t = (frequency - freq1) / (freq2 - freq1);
    float gainAtFrequency = catmullRom (t, gain0, gain1, gain2, gain3);
    
    return gainAtFrequency;
}

std::pair<float*, float*> Curve::frequencyResponse (int numPoints) const
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
