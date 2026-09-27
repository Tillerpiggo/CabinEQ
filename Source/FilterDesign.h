/*
  ==============================================================================

    FilterDesign.h

    Biquad coefficients for each band shape (RBJ Audio EQ Cookbook), and their
    magnitude response. The audio path and the graph both use these, so what the
    graph draws is exactly what the filters do at that sample rate.

  ==============================================================================
*/

#pragma once

#include <cmath>
#include "BandProfile.h"

namespace FilterDesign
{
    /// Coefficients normalised so that a0 = 1.
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    };

    /// The highest centre frequency a filter can have at this sample rate.
    inline double maxFrequency (double sampleRate)
    {
        return std::min ((double) Band::maxFreq, sampleRate * 0.49);
    }

    inline Biquad design (Band::Shape shape, double freq, double gainDb, double q, double sampleRate)
    {
        freq = juce::jlimit ((double) Band::minFreq, maxFrequency (sampleRate), freq);
        q = juce::jlimit ((double) Band::minQ, (double) Band::maxQ, q);

        const double w0 = juce::MathConstants<double>::twoPi * freq / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * q);
        const double A = std::pow (10.0, gainDb / 40.0);

        double b0 = 1, b1 = 0, b2 = 0, a0 = 1, a1 = 0, a2 = 0;

        switch (shape)
        {
            case Band::Shape::peak:
                b0 = 1.0 + alpha * A;
                b1 = -2.0 * cosW0;
                b2 = 1.0 - alpha * A;
                a0 = 1.0 + alpha / A;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha / A;
                break;

            case Band::Shape::lowShelf:
            {
                const double twoSqrtAAlpha = 2.0 * std::sqrt (A) * alpha;
                b0 = A * ((A + 1.0) - (A - 1.0) * cosW0 + twoSqrtAAlpha);
                b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW0);
                b2 = A * ((A + 1.0) - (A - 1.0) * cosW0 - twoSqrtAAlpha);
                a0 = (A + 1.0) + (A - 1.0) * cosW0 + twoSqrtAAlpha;
                a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW0);
                a2 = (A + 1.0) + (A - 1.0) * cosW0 - twoSqrtAAlpha;
                break;
            }

            case Band::Shape::highShelf:
            {
                const double twoSqrtAAlpha = 2.0 * std::sqrt (A) * alpha;
                b0 = A * ((A + 1.0) + (A - 1.0) * cosW0 + twoSqrtAAlpha);
                b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0);
                b2 = A * ((A + 1.0) + (A - 1.0) * cosW0 - twoSqrtAAlpha);
                a0 = (A + 1.0) - (A - 1.0) * cosW0 + twoSqrtAAlpha;
                a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW0);
                a2 = (A + 1.0) - (A - 1.0) * cosW0 - twoSqrtAAlpha;
                break;
            }

            case Band::Shape::lowCut:
                b0 = (1.0 + cosW0) / 2.0;
                b1 = -(1.0 + cosW0);
                b2 = (1.0 + cosW0) / 2.0;
                a0 = 1.0 + alpha;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha;
                break;

            case Band::Shape::highCut:
                b0 = (1.0 - cosW0) / 2.0;
                b1 = 1.0 - cosW0;
                b2 = (1.0 - cosW0) / 2.0;
                a0 = 1.0 + alpha;
                a1 = -2.0 * cosW0;
                a2 = 1.0 - alpha;
                break;
        }

        return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
    }

    inline Biquad design (const Band& band, double sampleRate)
    {
        return design (band.shape, band.freq, band.hasGain() ? band.ampl : 0.0, band.qFactor, sampleRate);
    }

    /// |H(e^jw)| in dB at this frequency.
    inline double magnitudeDb (const Biquad& c, double freq, double sampleRate)
    {
        const double w = juce::MathConstants<double>::twoPi * freq / sampleRate;
        const double cos1 = std::cos (w), sin1 = std::sin (w);
        const double cos2 = std::cos (2.0 * w), sin2 = std::sin (2.0 * w);

        const double numRe = c.b0 + c.b1 * cos1 + c.b2 * cos2;
        const double numIm = -(c.b1 * sin1 + c.b2 * sin2);
        const double denRe = 1.0 + c.a1 * cos1 + c.a2 * cos2;
        const double denIm = -(c.a1 * sin1 + c.a2 * sin2);

        const double magSq = (numRe * numRe + numIm * numIm) / (denRe * denRe + denIm * denIm);
        return 10.0 * std::log10 (std::max (magSq, 1.0e-20));
    }
}
