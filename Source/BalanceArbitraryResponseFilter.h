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
    
    void update (const Curve& curve, int fft_size) override;
};
