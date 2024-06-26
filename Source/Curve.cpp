/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const std::complex<float> Curve::valueAtFrequency (float frequency) const
{
    auto setPointFreqs = setPointManager.getSetPointFreqs();
    auto setPointGains = setPointManager.getSetPointGains();
    
    // Windowing (kinda poorly tho)
    
    if (frequency < 20)
    {
        return setPointGains.at (0) * (frequency / 20.0);
    }
    
    if (frequency > setPointFreqs.at(SetPointManager::NUM_SET_POINTS - 1))
    {
        return (22050.f - frequency) / 2050.f;
    }
    
    float setPointFreq1;
    float setPointGain1;
    float setPointFreq2;
    float setPointGain2;
    
    float setPointGain0;
    float setPointGain3;
    
    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
    {
        // If the frequency is the same, return the value of the set point
        if (frequency == setPointFreqs.at(i))
        {
            return setPointGains.at(i);
        }
        
        if (frequency < setPointFreqs.at(i))
        {
            setPointFreq1 = setPointFreqs.at(i-1); // There should always be a previous set point. The only way for there not to be one is if freq <= setPoints.at(0), but we already check those cases.
            setPointFreq2 = setPointFreqs.at(i);
            setPointGain1 = setPointGains.at(i-1); // Same reasoning as above.
            setPointGain2 = setPointGains.at(i);
            
            if (i > 1)
            {
                setPointGain0 = setPointGains.at(i-2);
            }
            else
            {
                setPointGain0 = setPointGain1;
            }
            
            if (i < SetPointManager::NUM_SET_POINTS - 1)
            {
                setPointGain3 = setPointGains.at(i+1);
            }
            else
            {
                setPointGain3 = setPointGain2;
            }
            
            break;
        }
    }
    
    // Interpolate using catmull-rom
    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
    float gainAtFrequency = catmullRom (t, setPointGain0, setPointGain1, setPointGain2, setPointGain3);
    
    // I tried the inverseFM, it sounds like shit... :(
//    std::cout << "gain at freq: " << gainAtFrequency << ", inverseFM: " << inverseFM.valueAtFrequency (frequency).real();
    return gainAtFrequency;// + std::complex<float>(0.2) * inverseFM.valueAtFrequency(frequency);//* factor;
}

// Returns the value along the curve in time (0 < t < 1), such that t is a linear
// mapping across frequencies. E.g. t=0 would be 20hz, t=0.5 would be ~10khz, and t=1 would be ~20khz.
const std::complex<float> Curve::valueAtTime (float t) const
{
    auto setPointFreqs = setPointManager.getSetPointFreqs();
    float minFreq = setPointFreqs.at(0);
    float maxFreq = setPointFreqs.at(setPointFreqs.size() - 1);
    
    // Scale linearly
    float freq = t * (maxFreq - minFreq) + minFreq;
    return valueAtFrequency(freq);
}

// Returns the value along the curve in normalized time (0 < t < 1), such that when plotted,
// it properly displays the logarithmic frequency response.

// NOTE: it's debatable whether this scaling logic really belongs in Curve or should stay in CurveComponent.
const std::complex<float> Curve::valueAtNormalizedTime (float t) const
{
    auto setPointFreqs = setPointManager.getSetPointFreqs();
    float minFreq = setPointFreqs.at(0);
    float maxFreq = setPointFreqs.at(setPointFreqs.size() - 1);
    
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

const std::complex<float> LinearCurve::valueAtFrequency (float frequency) const
{
    auto setPointGains = setPointManager.getSetPointGains();
    auto setPointFreqs = setPointManager.getSetPointFreqs();
    
    // Return 1 if the frequency is out of the 20hz-20000hz range
    if (frequency < 20.f || frequency > 20000.f)
    {
        return 1;
    }
    
    // Find the two set points this frequency lies between
    // (return the set point if it hits a set point exactly)
    
    float setPointFreq1;
    float setPointGain1;
    float setPointFreq2;
    float setPointGain2;

    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; i++)
    {
        // If the frequency is the same, return the value of the set point
        if (frequency == setPointFreqs.at(i))
        {
            return setPointGains.at(i);
        }
        
        if (frequency < setPointFreqs.at(i))
        {
            setPointFreq1 = setPointFreqs.at(i-1); // There should always be a previous set point. The only way for there not to be one is if freq <= setPoints.at(0), but we already check those cases.
            setPointFreq2 = setPointFreqs.at(i);
            setPointGain1 = setPointGains.at(i-1); // Same reasoning as above.
            setPointGain2 = setPointGains.at(i);
            break;
        }
    }
    
    // Interpolate using linear curve
    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
    float gainAtFrequency = t * (setPointGain2 - setPointGain1) + setPointGain1;
    
    return gainAtFrequency;
}
