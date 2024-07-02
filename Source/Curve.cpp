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
    float panAtFrequency = 0;
    float phaseAtFrequency = 0;
    
    if (pans.has_value())
    {
        panAtFrequency = interpolateValueAtFrequency (frequency, *pans.value());
    }
    
    if (phases.has_value())
    {
        phaseAtFrequency = interpolateValueAtFrequency (frequency, *phases.value());
    }
    
    float leftGain = juce::Decibels::decibelsToGain (-0.5 * panAtFrequency + amplitudeAtFrequency);
    float rightGain = juce::Decibels::decibelsToGain (0.5 * panAtFrequency + amplitudeAtFrequency);
    
    std::complex<float> leftVal = std::polar (leftGain, 0.0f);
    std::complex<float> rightVal = std::polar (rightGain, phaseAtFrequency);
    
    return { leftVal, rightVal };
}

// Returns the value along the curve in time (0 < t < 1), such that t is a linear
// mapping across frequencies. E.g. t=0 would be 20hz, t=0.5 would be ~10khz, and t=1 would be ~20khz.
const std::pair<std::complex<float>, std::complex<float>> Curve::valueAtTime (float t) const
{
    float minFreq = frequencies.at(0);
    float maxFreq = frequencies.at(frequencies.size() - 1);
    
    // Scale linearly
    float freq = t * (maxFreq - minFreq) + minFreq;
    return valueAtFrequency(freq);
}

// Returns the value along the curve in normalized time (0 < t < 1), such that when plotted,
// it properly displays the logarithmic frequency response.

// NOTE: it's debatable whether this scaling logic really belongs in Curve or should stay in CurveComponent.
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
    // We only need to calculate y
    float y = (pow(1.f - t, 3) * y0) + (3 * pow(1.f - t, 2) * t * y0) + (3*(1 - t)*pow(t, 2) * y1) + (pow(t, 3.f) * y1);
    return y;
}

const float Curve::interpolateValueAtFrequency (const float frequency, 
                                                const std::vector<CalibratedSetPoint>& values) const
{
    
    
    if (frequency < 10)
    {
        return values.at (0).estimatedValue();
    }
    
    if (frequency > frequencies.at(SetPointManager::NUM_SET_POINTS - 1))
    {
        return values.at (values.size() - 1).estimatedValue();
    }
    
    float freq1, freq2;
    float gain0, gain1, gain2, gain3;
    
    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
    {
        // If the frequency is the same, return the value of the set point
        if (frequency == frequencies.at(i))
        {
            return values.at(i).estimatedValue();
        }
        
        if (frequency < frequencies.at(i))
        {
            freq1 = frequencies.at(i - 1); // There should always be a previous set point. The only way for there not to be one is if freq <= setPoints.at(0), but we already check those cases.
            freq2 = frequencies.at(i);
            gain1 = values.at(i - 1).estimatedValue(); // Same reasoning as above.
            gain2 = values.at(i).estimatedValue();
            
            if (i > 1)
            {
                gain0 = values.at(i - 2).estimatedValue();
            }
            else
            {
                gain0 = gain1;
            }
            
            if (i < SetPointManager::NUM_SET_POINTS - 1)
            {
                gain3 = values.at(i + 1).estimatedValue();
            }
            else
            {
                gain3 = gain2;
            }
            
            break;
        }
    }
    
    // Interpolate using catmull-rom
    float t = (frequency - freq1) / (freq2 - freq1);
    float gainAtFrequency = catmullRom (t, gain0, gain1, gain2, gain3);
    
    return gainAtFrequency;
}
