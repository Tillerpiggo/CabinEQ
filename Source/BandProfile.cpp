/*
  ==============================================================================

    BandProfile.cpp
    Created: 25 Dec 2024 2:43:35pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BandProfile.h"

//==============================================
Band::Band()
    : id (0), freq (1000.0f), ampl (0.0f), bandwidth (1.0f), qFactor (bandwidthToQFactor (1.0f)),
      type (Type::both), shape (Shape::peak), enabled (true)
{}

Band::Band (int id, float freq, float ampl, float bandwidth, Type type, Shape shape, bool enabled)
    : id (id), freq (freq), ampl (ampl), bandwidth (bandwidth), qFactor (bandwidthToQFactor (bandwidth)),
      type (type), shape (shape), enabled (enabled)
{}

Band Band::withQ (int id, float freq, float ampl, float qFactor, Type type, Shape shape, bool enabled)
{
    Band band (id, freq, ampl, qFactorToBandwidth (qFactor), type, shape, enabled);
    band.qFactor = qFactor; // keep the exact Q rather than the round trip through bandwidth
    return band;
}

float Band::bandwidthToQFactor (float bandwidth)
{
    return std::sqrt (std::pow (2.0f, bandwidth)) / (std::pow (2.0f, bandwidth) - 1.0f);
}

float Band::qFactorToBandwidth (float qFactor)
{
    // Bandwidth (octaves) = log2((sqrt(4 * Q^2 + 1) + 1) / (sqrt(4 * Q^2 + 1) - 1))
    float sqrtTerm = std::sqrt (4.0f * qFactor * qFactor + 1.0f);
    return std::log2 ((sqrtTerm + 1.0f) / (sqrtTerm - 1.0f));
}

void Band::setQ (float newQ)
{
    qFactor = juce::jlimit (minQ, maxQ, newQ);
    bandwidth = qFactorToBandwidth (qFactor);
}

void Band::setBandwidth (float newBandwidth)
{
    setQ (bandwidthToQFactor (newBandwidth));
}

bool Band::hasGain() const
{
    return shape == Shape::peak || shape == Shape::lowShelf || shape == Shape::highShelf;
}

bool Band::appliesToChannel (int channel) const
{
    return type == Type::both
        || (type == Type::left && channel == 0)
        || (type == Type::right && channel == 1);
}

juce::String Band::shapeName (Shape shape)
{
    switch (shape)
    {
        case Shape::peak:      return "Bell";
        case Shape::lowShelf:  return "Low shelf";
        case Shape::highShelf: return "High shelf";
        case Shape::lowCut:    return "Low cut";
        case Shape::highCut:   return "High cut";
    }
    return {};
}

juce::String Band::typeName (Type type)
{
    switch (type)
    {
        case Type::both:  return "Stereo";
        case Type::left:  return "Left";
        case Type::right: return "Right";
    }
    return {};
}

//==============================================
BandProfile::BandProfile (std::vector<Band> bands, float volume)
    : bandsOn (true), curveOn (false), bands (std::move (bands)), volume (volume)
{}

BandProfile::BandProfile (std::vector<CurvePoint> points, float volume)
    : bandsOn (false), curveOn (true), volume (volume)
{
    setPoints (std::move (points));
}

void BandProfile::setPoints (std::vector<CurvePoint> newPoints)
{
    points = std::move (newPoints);
    shared.setPoints (points);

    pointsDiffer = false;
    for (const auto& point : points)
        pointsDiffer |= point.differsBetweenEars();
    leftShared.setPoints (getEarPoints (0));
    rightShared.setPoints (getEarPoints (1));
}

std::vector<CurvePoint> BandProfile::getEarPoints (int ear) const
{
    std::vector<CurvePoint> earPoints;
    for (const auto& point : points)
        earPoints.push_back (point.forEar (ear));
    return earPoints;
}

float BandProfile::sharedDbAt (float frequency, int ear) const
{
    return ear == 0 ? leftShared.dbAtFrequency (frequency) : ear == 1 ? rightShared.dbAtFrequency (frequency) : shared.dbAtFrequency (frequency);
}

const std::vector<CurvePoint>& BandProfile::getPoints (int layer) const
{
    if (split && layer == leftTweak)
        return leftTweakPoints;
    if (split && layer == rightTweak)
        return rightTweakPoints;
    return points;
}

void BandProfile::setSplit (std::vector<CurvePoint> newLeftTweak, std::vector<CurvePoint> newRightTweak)
{
    split = true;
    leftTweakPoints = std::move (newLeftTweak);
    rightTweakPoints = std::move (newRightTweak);
    leftResponse.setPoints (leftTweakPoints);
    rightResponse.setPoints (rightTweakPoints);
}

float BandProfile::curveDbAt (float frequency, int ear) const
{
    if (ear == -2 || ! earsDiffer())
        return shared.dbAtFrequency (frequency);

    const float left = leftShared.dbAtFrequency (frequency) + (split ? leftResponse.dbAtFrequency (frequency) : 0.0f);
    const float right = rightShared.dbAtFrequency (frequency) + (split ? rightResponse.dbAtFrequency (frequency) : 0.0f);
    return ear == 0 ? left : ear == 1 ? right : 0.5f * (left + right);
}

std::optional<CurvePoint> BandProfile::getPointWithId (int id, int layer) const
{
    for (const auto& point : getPoints (layer))
        if (point.id == id)
            return point;
    return std::nullopt;
}

const std::vector<Band>& BandProfile::getBands() const
{
    return bands;
}

std::optional<Band> BandProfile::getBandWithId (int id) const
{
    for (const auto& band : bands)
        if (band.id == id)
            return band;
    return std::nullopt;
}

float BandProfile::getVolume() const
{
    return volume;
}
