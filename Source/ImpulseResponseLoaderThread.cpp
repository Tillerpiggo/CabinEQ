/*
  ==============================================================================

    ImpulseResponseLoaderThread.cpp
    Created: 8 Sep 2024 9:33:26pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ImpulseResponseLoaderThread.h"
#include "ArbitraryResponseFilter.h"

void ImpulseResponseLoaderThread::run()
{
    filter.generateAndLoadImpulseResponse(amplCurve, panCurve, phaseCurve, fft_size);
}
