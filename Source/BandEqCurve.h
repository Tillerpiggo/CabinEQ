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
#include "FilterDesign.h"

/// The frequency response of a set of bands, for drawing the graph and for loudness matching.
/// Disabled bands don't count.
class BandEqCurve
{
public:
    BandEqCurve() = default;

    void updateWithBands (const std::vector<Band>& bands);
    void setSampleRate (double sampleRate);

    float dbAtFrequency (float frequency) const; // average of left and right
    float leftDbAtFrequency (float frequency) const;
    float rightDbAtFrequency (float frequency) const;
    float dbAtFrequencyForChannel (float frequency, int channel) const;
    float dbAtFrequencyForBand (const Band& band, float frequency) const;

    /// True if any enabled band only applies to one channel, so left and right differ.
    bool hasChannelSpecificBands() const;

    /// How much louder the EQ makes typical music, in dB, averaged over both channels.
    /// Auto gain applies the negative of this so that toggling the EQ doesn't change loudness.
    float loudnessChangeDb() const;

private:
    struct CachedBand
    {
        Band band;
        FilterDesign::Biquad coefficients;
    };

    void recalculate();

    std::vector<Band> bands;
    std::vector<CachedBand> cachedBands;
    double sampleRate = 48000.0;
};
