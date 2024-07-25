/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtFrequency (float frequency) const
{
    float amplitudeAtFrequency = interpolateValueAtFrequency (frequency, amplitudes);
    float panAtFrequency = interpolateValueAtFrequency (frequency, pans);
    float phaseAtFrequency = interpolatePhaseAtFrequency (frequency);
    
    float dbDifference = -4.5f * std::log2((frequency) / 1000.0f);
    
    if (frequency < 50.0f) dbDifference = -4.5f * std::log2(50.0f / 1000.0f);
    //if (frequency > 15000.0f) dbDifference = -4.5f * std::log2(15000.0f / 1000.0f);
    
    float leftGain = juce::Decibels::decibelsToGain (-0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference);
    float rightGain = juce::Decibels::decibelsToGain (0.5 * panAtFrequency + amplitudeAtFrequency - dbDifference);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, phaseAtFrequency);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtTime (float t) const
{
    return valueAtFrequency(t * 22050);
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtNormalizedTime (float t) const
{
    float minFreq = 50;//frequencies.at(0);
    float maxFreq = 22050;//frequencies.at(frequencies.size() - 1);
    
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(minFreq);
    float logMaxFreq = std::log(maxFreq);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    return valueAtFrequency(freq);
}

const float Curve::catmullRom (float t, float y0, float y1, float y2, float y3) const
{
    float y = 0.5 * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * pow(t, 2) + (-y0 + 3 * y1 - 3 * y2 + y3) * pow(t, 3));
    return y;
}

const float Curve::cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const
{
    float y = (pow(1.f - t, 3) * y0) + (3 * pow(1.f - t, 2) * t * y0) + (3 * (1 - t) * pow(t, 2) * y1) + (pow(t, 3.f) * y1);
    return y;
}

void Curve::setFactor (const float factor)
{
    this->factor = factor;
}

void Curve::setFrequencies(std::vector<float> frequencies)
{
    this->frequencies = frequencies;
}

void Curve::setAmplitudes(std::vector<float> amplitudes)
{
    this->amplitudes = amplitudes;
}

void Curve::setPhases(std::vector<float> phases)
{
    this->phases = phases;
}

void Curve::setPans(const std::vector<float>& pans)
{
    this->pans = pans;
}

void Curve::updateWithSetPoints (std::vector<SetPoint> setPoints)
{
    frequencies.clear();
    amplitudes.clear();
    pans.clear();
    phases.clear();
    for (SetPoint setPoint : setPoints)
    {
        frequencies.push_back (setPoint.frequency);
        amplitudes.push_back (setPoint.amplitude);
        pans.push_back (setPoint.pan);
        phases.push_back (0);
    }
}

const std::pair<float*, float*> Curve::getStereoImpulse (int fft_size) const
{
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
    auto freqResponse = frequencyResponse (numPoints);
    auto leftFreqResponse = freqResponse.first;
    auto rightFreqResponse = freqResponse.second;
    
    std::cout << "Freq response: " << std::endl;
    for (int i = 0; i < numPoints; ++i)
    {
        std::cout << leftFreqResponse[i] << " ";
    }
    std::cout << std::endl;

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
    size_t numPoints = frequencies.size();
    
    if (frequency < frequencies.at (0))
    {
        return values.at (0);
    }
    
    if (frequency > frequencies.at(numPoints - 1))
    {
        return values.at (numPoints - 1);
    }
    
    float freq1, freq2;
    float gain0, gain1, gain2, gain3;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        if (frequency == frequencies.at(i))
        {
            return values.at(i);
        }
        
        if (frequency < frequencies.at(i))
        {
            freq1 = frequencies.at(i - 1);
            freq2 = frequencies.at(i);
            gain1 = values.at(i - 1);
            gain2 = values.at(i);
            
            gain0 = (i > 1) ? values.at(i - 2) : gain1;
            gain3 = (i < numPoints - 1) ? values.at(i + 1) : gain2;
            
            break;
        }
    }
    
    float t = (frequency - freq1) / (freq2 - freq1);
    float gainAtFrequency = catmullRom(t, gain0, gain1, gain2, gain3);
    
    return gainAtFrequency;
}

const float Curve::interpolatePhaseAtFrequency (const float frequency) const
{
    size_t numPoints = frequencies.size();
    
    if (frequency < frequencies.at(0))
    {
        return phases.at(0);
    }
    
    if (frequency > frequencies.at(numPoints - 1))
    {
        return phases.at(phases.size() - 1);
    }
    
    float freq1, freq2;
    float gain0, gain1, gain2, gain3;
    
    for (size_t i = 0; i < numPoints; ++i)
    {
        if (frequency == frequencies.at(i))
        {
            return phases.at(i);
        }
        
        if (frequency < frequencies.at(i))
        {
            freq1 = frequencies.at(i - 1);
            freq2 = frequencies.at(i);
            gain1 = phases.at(i - 1);
            gain2 = phases.at(i);
            
            gain0 = (i > 1) ? phases.at(i - 2) : gain1;
            gain3 = (i < numPoints - 1) ? phases.at(i + 1) : gain2;
            
            break;
        }
    }
    
    float t = (frequency - freq1) / (freq2 - freq1);
    float gainAtFrequency = catmullRom(t, gain0, gain1, gain2, gain3);
    
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
