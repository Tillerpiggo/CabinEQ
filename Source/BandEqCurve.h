/*
  ==============================================================================

    BandEqCurve.h
    Created: 12 Oct 2024 8:26:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"

/// Class helpful for rendering the eq curve formed by parametric bands
class BandEqCurve
{
public:
    BandEqCurve() = default;
    virtual ~BandEqCurve() = default;
    
    const float dbAtFrequency (float frequency) const;
    const float dbAtFrequencyForBand (Band band, float frequency) const;
    void updateWithBands (std::vector<Band> bands);
    
protected:
    std::vector<Band> bands;
};
