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
    
    if (setPointMap->empty())
    {
        std::cout << "WARNING: GETTING VALUE FROM CURVE BEFORE SETTING SETPOINTMAP" << std::endl;
        return 0.0f;
    }
    
    // Windowing (kinda poorly tho)
    
    if (frequency < 10.f)
    {
        return setPointMap->begin()->second.estimatedValue();
    }
    
    if (frequency > 20000.f)
    {
        return setPointMap->end()->second.estimatedValue();
    }
    
    // Find the two set points this frequency lies between
    // (return the set point if it hits a set point exactly)
    float setPointFreq1;
    float setPointGain1;
    float setPointFreq2;
    float setPointGain2;
    
    float setPointGain0;
    float setPointGain3;
    
    auto it = setPointMap->begin();
    auto prev_it = setPointMap->end();

    for (; it != setPointMap->end(); ++it) {
        const auto& [freq, gain] = *it;

        // If the frequency is the same, return the value of the set point
        if (frequency == freq) 
        {
            return gain.estimatedValue();
        }

        if (frequency < freq) 
        {
            if (prev_it != setPointMap->end())
            {
                setPointFreq1 = prev_it->first;
                setPointGain1 = prev_it->second.estimatedValue();
            }
            setPointFreq2 = it->first;
            setPointGain2 = it->second.estimatedValue();

            if (prev_it != setPointMap->begin() && prev_it != setPointMap->end()) 
            {
                auto prev_prev_it = std::prev(prev_it);
                setPointGain0 = prev_prev_it->second.estimatedValue();
            } 
            else
            {
                setPointGain0 = setPointGain1;
            }

            auto next_it = std::next(it);
            if (next_it != setPointMap->end()) 
            {
                setPointGain3 = next_it->second.estimatedValue();
            } 
            else
            {
                setPointGain3 = setPointGain2;
            }

            break;
        }

        prev_it = it;
    }
    
    // Interpolate using catmull-rom
    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
    float gainAtFrequency = catmullRom (t, setPointGain0, setPointGain1, setPointGain2, setPointGain3);
    
    return gainAtFrequency * factor;
}

// Returns the value along the curve in time (0 < t < 1), such that t is a linear
// mapping across frequencies. E.g. t=0 would be 20hz, t=0.5 would be ~10khz, and t=1 would be ~20khz.
const std::complex<float> Curve::valueAtTime (float t) const
{
    if (setPointMap->empty()) {
        std::cout << "WARNING: TRYING TO ACCESS VALUE IN CURVE BEFORE SETTING SET POINTS" << std::endl;
        return std::complex<float>(0.0f, 0.0f);
    }
    
    float minFreq = setPointMap->begin()->first;
    float maxFreq = setPointMap->rbegin()->first;
    
    // Scale linearly
    float freq = t * (maxFreq - minFreq) + minFreq;
    return valueAtFrequency(freq);
}

// Returns the value along the curve in normalized time (0 < t < 1), such that when plotted,
// it properly displays the logarithmic frequency response.

// NOTE: it's debatable whether this scaling logic really belongs in Curve or should stay in CurveComponent.
const std::complex<float> Curve::valueAtNormalizedTime (float t) const
{
    if (setPointMap->empty()) {
        std::cout << "WARNING: TRYING TO ACCESS VALUE IN CURVE BEFORE SETTING SET POINTS" << std::endl;
        return std::complex<float>(0.0f, 0.0f);
    }
    
    float minFreq = setPointMap->begin()->first;
    float maxFreq = setPointMap->rbegin()->first;
    
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(minFreq);
    float logMaxFreq = std::log(maxFreq);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    return valueAtFrequency(freq);
}

const float Curve::catmullRom (float t, float y0, float y1, float y2, float y3) const
{
    float y = 0.5 * ((2.f*y1) + (-y0 + y2) * t + (2.f*y0 - 5.f*y1 + 4.f*y2 - y3) * pow(t, 2) + (-y0 + 3*y1 - 3*y2 + y3) * pow(t, 3));
    
    return y;
}

const float Curve::cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const
{
    // We only need to calculate y
    float y = (pow(1.f-t, 3) * y0) + (3*pow(1.f-t, 2) * t * y0) + (3*(1-t)*pow(t, 2) * y1) + (pow(t, 3.f) * y1);
    return y;
}

//const std::complex<float> LinearCurve::valueAtFrequency (float frequency) const
//{
//    // Return 1 if the frequency is out of the 20hz-20000hz range
//    if (frequency < 20.f || frequency > 20000.f)
//    {
//        return 1;
//    }
//    
//    // Find the two set points this frequency lies between
//    // (return the set point if it hits a set point exactly)
//    auto setPoints = setPointLayout.getSetPoints();
//    float setPointFreq1;
//    float setPointGain1;
//    float setPointFreq2;
//    float setPointGain2;
//    
//    for (int i = 0; i < setPoints.size(); i++)
//    {
//        // If the frequency is the same, return the value of the set point
//        if (frequency == setPoints.at(i))
//        {
//            return setPointGains.at(i);
//        }
//        
//        if (frequency < setPoints.at(i))
//        {
//            setPointFreq1 = setPoints.at(i-1); // There should always be a previous set point. The only way for there not to be one is if freq <= setPoints.at(0), but we already check those cases.
//            setPointFreq2 = setPoints.at(i);
//            setPointGain1 = setPointGains.at(i-1); // Same reasoning as above.
//            setPointGain2 = setPointGains.at(i);
//            break;
//        }
//    }
//    
//    // Interpolate using linear curve
//    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
//    float gainAtFrequency = t * (setPointGain2 - setPointGain1) + setPointGain1;
//    
//    
//    
//    return gainAtFrequency;
//}

const std::complex<float> LinearCurve::valueAtFrequency(float frequency) const 
{
    // Return 1 if the frequency is out of the 20Hz-20000Hz range
    if (frequency < 20.0f || frequency > 20000.0f) 
    {
        return 1.0f;
    }
    
    float setPointFreq1 = 0.0f;
    float setPointGain1 = 0.0f;
    float setPointFreq2 = 0.0f;
    float setPointGain2 = 0.0f;

    auto it = setPointMap->begin();
    auto prev_it = setPointMap->end();

    for (; it != setPointMap->end(); ++it) 
    {
        float freq = it->first;
        float gain = it->second.estimatedValue();

        // If the frequency is the same, return the value of the set point
        if (frequency == freq) 
        {
            return gain;
        }

        if (frequency < freq) 
        {
            if (prev_it != setPointMap->end()) 
            {
                setPointFreq1 = prev_it->first;
                setPointGain1 = prev_it->second.estimatedValue();
            }
            setPointFreq2 = it->first;
            setPointGain2 = it->second.estimatedValue();
            break;
        }

        prev_it = it;
    }

    // If we haven't found a range, return 1.0f
    if (setPointFreq1 == 0.0f && setPointGain1 == 0.0f && setPointFreq2 == 0.0f && setPointGain2 == 0.0f) 
    {
        return 1.0f;
    }

    // Interpolate using a linear curve
    float t = (frequency - setPointFreq1) / (setPointFreq2 - setPointFreq1);
    float gainAtFrequency = t * (setPointGain2 - setPointGain1) + setPointGain1;

    return gainAtFrequency;
}
