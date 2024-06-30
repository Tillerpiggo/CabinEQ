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
        for (int i = 0; i < numPoints; i += 2)
        {
            float t = static_cast<float> (i) / static_cast<float> (numPoints);
            
            auto [leftVal, rightVal] = curve.valueAtTime (t);
            
            leftFreqResponse[i] = leftVal.real();
            rightFreqResponse[i] = rightVal.real();
            leftFreqResponse[i + 1] = leftVal.imag();
            rightFreqResponse[i + 1] = rightVal.imag();
        }
        
        return { leftFreqResponse, rightFreqResponse };
    }
};
