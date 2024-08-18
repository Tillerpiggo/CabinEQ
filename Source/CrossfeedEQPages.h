/*
  ==============================================================================

    CrossfeedEQPages.h
    Created: 17 Aug 2024 6:23:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEQPage.h"

class LeftCurveEQPage   : public CabinEQPage
{
    using CabinEQPage::CabinEQPage;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
};

class RightCurveEQPage   : public CabinEQPage
{
    using CabinEQPage::CabinEQPage;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
};
