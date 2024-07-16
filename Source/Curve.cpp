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
    
    // Figure out delta of dB from reference tone
    const float referenceFrequency = 1000.0;
    const float slope = -4.5;
    float octaves = std::log2(frequency / referenceFrequency);
    float dbDifference = octaves * slope;
    
    // Figure out delta of fletcher munson curves at that frequency and dB difference
    const float referenceDB = 83.0;
    float inverseFMDiff = inverseFM.valueAtFrequency (frequency, referenceDB + dbDifference) - inverseFM.valueAtFrequency (frequency, referenceDB); // for measurement from -3db slope
    inverseFMDiff -= dbDifference;
    
    
    if (frequency < 20)
    {
        dbDifference = 0.0f;
        inverseFMDiff = 0.0f;
    }
    
    float leftGain = juce::Decibels::decibelsToGain (-0.5 * panAtFrequency + amplitudeAtFrequency + inverseFMDiff);
    float rightGain = juce::Decibels::decibelsToGain (0.5 * panAtFrequency + amplitudeAtFrequency + inverseFMDiff);
    
    std::complex<float> leftVal = std::polar(leftGain, 0.0f);
    std::complex<float> rightVal = std::polar(rightGain, phaseAtFrequency);
    
    return { leftVal, rightVal };
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtTime (float t) const
{
//    float minFreq = frequencies.at(0);
//    float maxFreq = frequencies.at(frequencies.size() - 1);
//
//    // Scale linearly
//    float freq = t * (maxFreq - minFreq) + minFreq;
    return valueAtFrequency(t * 22050);
}

const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtNormalizedTime (float t) const
{
    float minFreq = frequencies.at(0);
    float maxFreq = frequencies.at(frequencies.size() - 1);
    
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
