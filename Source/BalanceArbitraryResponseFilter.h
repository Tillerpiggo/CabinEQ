/*
  ==============================================================================

    BalanceArbitraryResponseFilter.h
    Created: 16 Jun 2024 9:35:44am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BalanceArbitraryResponseFilter.h"
#include "ArbitraryResponseFilter.h"

class BalanceArbitraryResponseFilter : public ArbitraryResponseFilter
{
public:
    using ArbitraryResponseFilter::ArbitraryResponseFilter;
    
    std::pair<float*, float*> frequencyResponse (const Curve& curve, int numPoints) override
    {
        float* leftFreqResponse = new float[2 * numPoints];
        float* rightFreqResponse = new float[2 * numPoints];
        for (int i = 0; i < 2 * numPoints; i++)
        {
            float t = static_cast<float>(i) / (2 * numPoints);
            
            std::complex val = curve.valueAtTime (t);
            
            if (i % 2 == 0)
            {
                leftFreqResponse[i] = juce::Decibels::decibelsToGain (-0.5 * val.real());
                rightFreqResponse[i] = juce::Decibels::decibelsToGain (0.5 * val.real());
            }
            else
            {
                leftFreqResponse[i] = val.imag();
                rightFreqResponse[i] = val.imag();
            }
        }
        
        return { leftFreqResponse, rightFreqResponse };
    }
};
