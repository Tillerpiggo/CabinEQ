/*
  ==============================================================================

    CrossfeedEQPages.cpp
    Created: 17 Aug 2024 6:23:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CrossfeedEQPages.h"

void LeftCurveEQPage::startPlayingValueAt (float freq, float ampl)
{
    processor.startCalibratingEQNodeWithLeftCrossfeed (EQNode (-1, freq, ampl, 0.0f), 0, 0);
}

void LeftCurveEQPage::playValueAt (float freq, float ampl)
{
    processor.updateCalibratingEQNodeWithRightCrossfeed (EQNode (-1, freq, ampl, 0.0f), 0, 0);
}

void RightCurveEQPage::startPlayingValueAt (float freq, float ampl)
{
    processor.startCalibratingEQNodeWithLeftCrossfeed (EQNode (-1, freq, ampl, 0.0f), 0, 0);
}

void RightCurveEQPage::playValueAt (float freq, float ampl)
{
    processor.updateCalibratingEQNodeWithRightCrossfeed (EQNode (-1, freq, ampl, 0.0f), 0, 0);
}
