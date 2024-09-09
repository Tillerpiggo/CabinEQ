/*
  =============================================================================

    ImpulseResponseLoaderThread.
    Created: 8 Sep 2024 9:29:03p
    Author:  Tyler Ge

  =============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class ArbitraryResponseFilter;

class ImpulseResponseLoaderThread : public juce::Thread
{
public:
    ImpulseResponseLoaderThread(ArbitraryResponseFilter& filter, Curve& amplCurve, Curve& panCurve, Curve& phaseCurve, int fft_size)
        : juce::Thread("ImpulseResponseLoader"),
          filter(filter),
          amplCurve(amplCurve),
          panCurve(panCurve),
          phaseCurve(phaseCurve),
          fft_size(fft_size)
    {
    }

    void run() override;

private:
    ArbitraryResponseFilter& filter;
    Curve& amplCurve;
    Curve& panCurve;
    Curve& phaseCurve;
    int fft_size;
};

