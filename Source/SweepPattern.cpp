/*
  ==============================================================================

    SweepPattern.cpp
    Created: 20 Oct 2024 11:17:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

//#include "SweepPattern.h"
//
//SweepPattern::SweepPattern (float centerFreq, float bandwidth, float durationInSeconds, float sampleRate)
//    : centerFreq (centerFreq), bandwidth (bandwidth), sampleRate (sampleRate), durationInSeconds (durationInSeconds),
//      idx (0), cycleLen (durationInSeconds * sampleRate), currFreq (centerFreq)
//{}
//
//float SweepPattern::getNextFreq()
//{
//    // Linear
//    // Calculate the normalized time (0 to 1) within the cycle
//    float normalizedTime = static_cast<float>(idx) / cycleLen;
//
//    // Determine the direction of the sweep (0 to 1 for upward, 1 to 0 for downward)
//    float sweepDirection = (idx < cycleLen / 2) ? normalizedTime * 2.0f : 2.0f - normalizedTime * 2.0f;
//
//    // Map the sweep direction to the logarithmic frequency range
//    float logCenterFreq = std::log2(centerFreq);
//    float logMinFreq = logCenterFreq - bandwidth / 2.0f;
//    float logMaxFreq = logCenterFreq + bandwidth / 2.0f;
//    float logCurrFreq = juce::jmap(sweepDirection, 0.0f, 1.0f, logMinFreq, logMaxFreq);
//    currFreq = std::pow(2.0f, logCurrFreq);
//
//    // Increment the index and wrap around if it reaches the cycle length
//    idx = (idx + 1) % cycleLen;
//
//    return currFreq;
//}
//
//float SweepPattern::getCurrFreq() const
//{
//    return currFreq;
//}

#include "SweepPattern.h"

SweepPattern::SweepPattern (const std::vector<std::pair<float, float>>& points, float durationInSeconds, float sampleRate)
    : points (points), sampleRate (sampleRate), durationInSeconds (durationInSeconds),
      currFreq (points[0].first), currPan (points[0].second),
      currSegment(0), segmentSampleIdx(0)
{
    // Ensure at least 2 points
    jassert(points.size() >= 2 && "SweepPattern requires at least 2 points");

    totalSegments = points.size();
    cycleLen = static_cast<int>(durationInSeconds * sampleRate);

    // Calculate samples per segment for even distribution
    int baseSamplesPerSegment = cycleLen / totalSegments;
    int remainderSamples = cycleLen % totalSegments;

    segmentSampleCounts.resize(totalSegments, baseSamplesPerSegment);

    // Distribute any remaining samples across the first few segments
    for (int i = 0; i < remainderSamples; ++i)
        segmentSampleCounts[i]++;

    // Recalculate cycle length
    cycleLen = 0;
    for (int count : segmentSampleCounts)
        cycleLen += count;

    // Initialize the first segment
    segmentStartFreq = points[0].first;
    int nextPointIndex = (1) % points.size();
    segmentEndFreq = points[nextPointIndex].first;

    segmentStartPan = points[0].second;
    segmentEndPan = points[nextPointIndex].second;

    idx = 0;
    samplesPerSegment = segmentSampleCounts[0];
}

void SweepPattern::advanceSegment()
{
    currSegment++;

    // Wrap around if we've reached the end
    if (currSegment >= totalSegments)
        currSegment = 0;

    // Update segment start and end points
    segmentStartFreq = points[currSegment].first;
    int nextPointIndex = (currSegment + 1) % points.size();
    segmentEndFreq = points[nextPointIndex].first;

    segmentStartPan = points[currSegment].second;
    segmentEndPan = points[nextPointIndex].second;

    segmentSampleIdx = 0;
    samplesPerSegment = segmentSampleCounts[currSegment];
}

std::pair<float, float> SweepPattern::getNextFrequencyAndPan()
{
    // Calculate interpolation factor
    float t = static_cast<float>(segmentSampleIdx) / samplesPerSegment;

    // Logarithmic interpolation for frequency
    float startLogFreq = std::log2(segmentStartFreq);
    float endLogFreq = std::log2(segmentEndFreq);
    float currentLogFreq = juce::jmap(t, startLogFreq, endLogFreq);
    currFreq = std::pow(2.0f, currentLogFreq);

    // Linear interpolation for pan
    currPan = juce::jmap(t, segmentStartPan, segmentEndPan);

    // Advance indices
    segmentSampleIdx++;
    idx++;

    // Move to next segment if necessary
    if (segmentSampleIdx >= samplesPerSegment)
        advanceSegment();

    // Wrap around the cycle
    if (idx >= cycleLen)
        idx = 0;
    
    return { currFreq, currPan };
}

std::pair<float, float> SweepPattern::getCurrFrequencyAndPan() const
{
    return { currFreq, currPan };
}

SweepPattern SweepPattern::withPan (float pan)
{
    std::vector<std::pair<float, float>> pannedPoints;
    
    for (const auto& point : points)
    {
        pannedPoints.push_back ({ point.first, pan });
    }
    
    return SweepPattern (pannedPoints, durationInSeconds, sampleRate);
}
