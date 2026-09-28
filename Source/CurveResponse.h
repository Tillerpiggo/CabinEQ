/*
  ==============================================================================

    CurveResponse.h

    The "Curve" way of EQing: you place points, and the response is a smooth
    curve that goes through every one of them (rather than bands adding up).

    It's a monotone cubic (Fritsch-Carlson / PCHIP) in log-frequency: it passes
    through each point exactly and never overshoots between two, so there are
    no surprise bumps. It levels off at the first and last points and stays
    flat beyond them.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>
#include <vector>

struct CurvePoint
{
    int id = 0;
    float freq = 1000.0f;
    float gain = 0.0f; // dB

    static constexpr float minFreq = 20.0f, maxFreq = 20000.0f;
    static constexpr float minGain = -30.0f, maxGain = 30.0f;
};

class CurveResponse
{
public:
    CurveResponse() = default;
    explicit CurveResponse (std::vector<CurvePoint> points) { setPoints (std::move (points)); }

    void setPoints (std::vector<CurvePoint> newPoints)
    {
        points = std::move (newPoints);
        std::sort (points.begin(), points.end(), [] (const CurvePoint& a, const CurvePoint& b) { return a.freq < b.freq; });

        const size_t n = points.size();
        xs.resize (n);
        ys.resize (n);
        slopes.assign (n, 0.0);
        for (size_t i = 0; i < n; ++i)
        {
            xs[i] = std::log2 ((double) std::max (1.0f, points[i].freq));
            ys[i] = points[i].gain;
        }

        // Fritsch-Carlson slopes: zero where the curve turns round (so it doesn't overshoot), a weighted
        // harmonic mean elsewhere, and zero at the ends so it levels off into the flat beyond them
        for (size_t i = 1; i + 1 < n; ++i)
        {
            const double h0 = xs[i] - xs[i - 1], h1 = xs[i + 1] - xs[i];
            if (h0 <= 1.0e-9 || h1 <= 1.0e-9)
                continue;
            const double d0 = (ys[i] - ys[i - 1]) / h0, d1 = (ys[i + 1] - ys[i]) / h1;
            if (d0 * d1 <= 0.0)
                continue;
            const double w0 = 2.0 * h1 + h0, w1 = h1 + 2.0 * h0;
            slopes[i] = (w0 + w1) / (w0 / d0 + w1 / d1);
        }
    }

    const std::vector<CurvePoint>& getPoints() const { return points; }

    float dbAtFrequency (float frequency) const
    {
        const size_t n = points.size();
        if (n == 0)
            return 0.0f;

        const double x = std::log2 ((double) std::max (1.0f, frequency));
        if (n == 1 || x <= xs.front())
            return (float) ys.front();
        if (x >= xs.back())
            return (float) ys.back();

        // Find the segment, then the cubic Hermite between its two points
        const size_t i = (size_t) (std::upper_bound (xs.begin(), xs.end(), x) - xs.begin()) - 1;
        const double h = xs[i + 1] - xs[i];
        if (h <= 1.0e-9)
            return (float) ys[i + 1];
        const double t = (x - xs[i]) / h, t2 = t * t, t3 = t2 * t;
        return (float) ((2 * t3 - 3 * t2 + 1) * ys[i] + (t3 - 2 * t2 + t) * h * slopes[i]
                        + (-2 * t3 + 3 * t2) * ys[i + 1] + (t3 - t2) * h * slopes[i + 1]);
    }

    /// How much louder the curve makes typical music, in dB, weighted the same way as for bands
    float loudnessChangeDb() const
    {
        if (points.empty())
            return 0.0f;
        return loudnessChangeDb ([this] (float frequency) { return dbAtFrequency (frequency); });
    }

    /// How much louder a response makes music, weighted to where music has most of its energy
    static float loudnessChangeDb (const std::function<float (float)>& dbAt)
    {
        constexpr int numPoints = 256;
        const double logLow = std::log (20.0), logHigh = std::log (16000.0);
        const double logFullLow = std::log (100.0), logFullHigh = std::log (5000.0);
        double power = 0.0, weights = 0.0;
        for (int i = 0; i < numPoints; ++i)
        {
            const double logFreq = logLow + (logHigh - logLow) * (i + 0.5) / numPoints;
            double weight = 1.0;
            if (logFreq < logFullLow)       weight = (logFreq - logLow) / (logFullLow - logLow);
            else if (logFreq > logFullHigh) weight = (logHigh - logFreq) / (logHigh - logFullHigh);
            power += weight * std::pow (10.0, dbAt ((float) std::exp (logFreq)) / 10.0);
            weights += weight;
        }
        return (float) (10.0 * std::log10 (power / weights));
    }

    /// Points that trace another response (say, some bands) closely, using as few as it can:
    /// samples it finely, then drops whichever points it can do without, staying within `tolerance` dB.
    static std::vector<CurvePoint> tracing (std::function<float (float)> dbAt, float tolerance = 0.3f)
    {
        // Start from every twelfth of an octave
        std::vector<CurvePoint> candidate;
        for (int step = 0; step <= 120; ++step)
        {
            const float freq = 20.0f * std::pow (1000.0f, (float) step / 120.0f);
            candidate.push_back ({ step, freq, juce::jlimit (CurvePoint::minGain, CurvePoint::maxGain, dbAt (freq)) });
        }

        // Checked between the samples, against the real thing
        constexpr int numChecks = 240;
        std::array<float, numChecks> checkFreqs {}, targets {};
        for (int i = 0; i < numChecks; ++i)
        {
            checkFreqs[(size_t) i] = 20.0f * std::pow (1000.0f, (i + 0.5f) / (float) numChecks);
            targets[(size_t) i] = juce::jlimit (CurvePoint::minGain, CurvePoint::maxGain, dbAt (checkFreqs[(size_t) i]));
        }

        auto worstError = [&] (const std::vector<CurvePoint>& trial)
        {
            CurveResponse curve (trial);
            float worst = 0.0f;
            for (int i = 0; i < numChecks; ++i)
                worst = std::max (worst, std::abs (curve.dbAtFrequency (checkFreqs[(size_t) i]) - targets[(size_t) i]));
            return worst;
        };

        // Very sharp filters can't be followed exactly even by every sample, so allow for that
        tolerance = std::max (tolerance, worstError (candidate) + 0.5f * tolerance);

        // Keep the ends; drop the interior point that matters least, while the result stays close
        for (bool dropped = true; dropped && candidate.size() > 2;)
        {
            dropped = false;
            size_t best = 0;
            float bestError = tolerance;
            for (size_t i = 1; i + 1 < candidate.size(); ++i)
            {
                auto without = candidate;
                without.erase (without.begin() + (long) i);
                const float error = worstError (without);
                if (error <= bestError)
                {
                    bestError = error;
                    best = i;
                }
            }
            if (best > 0)
            {
                candidate.erase (candidate.begin() + (long) best);
                dropped = true;
            }
        }

        // A flat response doesn't need any points at all
        if (std::all_of (candidate.begin(), candidate.end(), [] (const CurvePoint& p) { return std::abs (p.gain) < 0.05f; }))
            return {};

        for (size_t i = 0; i < candidate.size(); ++i)
            candidate[i].id = (int) i;
        return candidate;
    }

private:
    std::vector<CurvePoint> points;
    std::vector<double> xs, ys, slopes;
};
