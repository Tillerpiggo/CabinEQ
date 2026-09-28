/*
  ==============================================================================

    BandProfile.h
    Created: 10 Oct 2024 7:05:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CurveResponse.h"

/// One parametric EQ band.
class Band
{
public:
    /// Which channels the band applies to. Stored as "bandtype" in saved profiles.
    enum class Type : int
    {
        both = 0,
        left = 1,
        right = 2
    };

    /// The filter's shape. Stored as "shape"; profiles saved before shapes existed are all peaks.
    enum class Shape : int
    {
        peak = 0,
        lowShelf = 1,
        highShelf = 2,
        lowCut = 3,
        highCut = 4
    };

    Band();
    Band (int id, float freq, float ampl, float bandwidth, Type type, Shape shape = Shape::peak, bool enabled = true);
    static Band withQ (int id, float freq, float ampl, float qFactor, Type type, Shape shape = Shape::peak, bool enabled = true);
    static float bandwidthToQFactor (float bandwidth);
    static float qFactorToBandwidth (float qFactor);

    void setQ (float newQ);
    void setBandwidth (float newBandwidth);

    /// Cuts have no gain, so their gain is ignored everywhere.
    bool hasGain() const;
    /// Whether the band is audible on this channel (0 = left, 1 = right).
    bool appliesToChannel (int channel) const;

    static juce::String shapeName (Shape shape);
    static juce::String typeName (Type type);

    static constexpr float minFreq = 10.0f;
    static constexpr float maxFreq = 22000.0f;
    static constexpr float minGain = -30.0f;
    static constexpr float maxGain = 30.0f;
    static constexpr float minQ = 0.1f;
    static constexpr float maxQ = 30.0f;
    static constexpr float defaultQ = 1.41f; // one octave for a peak
    static constexpr float defaultCutQ = 0.7071f; // Butterworth

    int id;
    float freq;
    float ampl; // gain in dB
    float bandwidth; // in octaves
    float qFactor;
    Type type;
    Shape shape;
    bool enabled;
};

/// A profile's EQ as a plain value for the DSP and the UI: either bands (IIR filters that add up), or a curve
/// through points (played by a FIR filter), and a preamp.
class BandProfile
{
public:
    enum class Mode { bands, curve };

    BandProfile() = default;
    BandProfile (std::vector<Band> bands, float volume);
    BandProfile (std::vector<CurvePoint> points, float volume); // a curve profile

    Mode getMode() const { return mode; }
    bool isCurve() const { return mode == Mode::curve; }

    const std::vector<Band>& getBands() const;
    std::optional<Band> getBandWithId (int id) const;
    const std::vector<CurvePoint>& getPoints() const { return points; }
    std::optional<CurvePoint> getPointWithId (int id) const;
    float getVolume() const; // preamp, in dB

    // Both kinds are kept, so switching back and forth loses nothing
    void setMode (Mode newMode) { mode = newMode; }
    void setPoints (std::vector<CurvePoint> newPoints) { points = std::move (newPoints); }
    void setBands (std::vector<Band> newBands) { bands = std::move (newBands); }

private:
    Mode mode = Mode::bands;
    std::vector<Band> bands;
    std::vector<CurvePoint> points;
    float volume = 0.0f;
};
