/*
  ==============================================================================

    FractalPattern.cpp
    Created: 9 Nov 2024 11:04:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FractalPattern.h"

FractalPattern::FractalPattern(int complexity)
    : complexity(complexity)
{
    gridSize = 1 << complexity; // M = 2^N
    numPoints = gridSize * gridSize;

    generateCurve();
    computeSegmentLengths();
}

void FractalPattern::generateCurve()
{
    points.clear();

    // Structure to hold center coordinates and Morton code
    struct CenterPoint
    {
        float x;
        float y;
        uint32_t mortonCode;
    };

    std::vector<CenterPoint> centers;
    centers.reserve(numPoints);

    // Generate centers and compute Morton codes
    for (uint32_t i = 0; i < (uint32_t)gridSize; ++i)
    {
        for (uint32_t j = 0; j < (uint32_t)gridSize; ++j)
        {
            float x = (i + 0.5f) / gridSize;
            float y = (j + 0.5f) / gridSize;
            uint32_t code = mortonEncode2D(i, j);
            centers.push_back({ x, y, code });
        }
    }

    // Sort centers based on Morton codes to get traversal order
    std::sort(centers.begin(), centers.end(), [](const CenterPoint& a, const CenterPoint& b) {
        return a.mortonCode < b.mortonCode;
    });

    // Map centers to frequency and pan
    for (const auto& pt : centers)
    {
        float x_norm = pt.x; // [0, 1]
        float y_norm = pt.y; // [0, 1]

        // Map x_norm to frequency using logarithmic scale
        float f_min = 20.0f;      // Minimum frequency (Hz)
        float f_max = 20000.0f;   // Maximum frequency (Hz)
        float frequency = f_min * powf(f_max / f_min, x_norm);

        // Map y_norm to pan [-1, 1]
        float pan = y_norm * 2.0f - 1.0f;

        points.emplace_back(frequency, pan);
    }
}

uint32_t FractalPattern::mortonEncode2D(uint32_t x, uint32_t y)
{
    uint32_t answer = 0;
    for (uint32_t i = 0; i < sizeof(uint32_t) * 4; ++i)
    {
        answer |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
    }
    return answer;
}

void FractalPattern::computeSegmentLengths()
{
    segmentLengths.clear();
    totalLength = 0.0f;
    size_t N = points.size();

    // Precompute lengths between consecutive points
    for (size_t i = 0; i < N; ++i)
    {
        const auto& p0 = points[i];
        const auto& p1 = points[(i + 1) % N]; // Wrap around

        // Normalize frequency and pan to [0, 1] for distance calculation
        float freq0 = (log10f(p0.first) - log10f(20.0f)) / (log10f(20000.0f) - log10f(20.0f));
        float pan0 = (p0.second + 1.0f) / 2.0f;

        float freq1 = (log10f(p1.first) - log10f(20.0f)) / (log10f(20000.0f) - log10f(20.0f));
        float pan1 = (p1.second + 1.0f) / 2.0f;

        float dx = freq1 - freq0;
        float dy = pan1 - pan0;

        float length = sqrtf(dx * dx + dy * dy);
        segmentLengths.push_back(length);
        totalLength += length;
    }
}

std::pair<float, float> FractalPattern::getFrequencyAndPanAtTime(float time)
{
    // Ensure time is within [0, 1)
    float t_mod = fmodf(time, 1.0f);
    if (t_mod < 0.0f)
        t_mod += 1.0f;

    // Map time to position along the total length of the curve
    float targetLength = t_mod * totalLength;

    // Find the segment corresponding to targetLength
    float accumulatedLength = 0.0f;
    size_t N = points.size();

    size_t segmentIndex = 0;
    for (; segmentIndex < N; ++segmentIndex)
    {
        float segLength = segmentLengths[segmentIndex];
        if (accumulatedLength + segLength >= targetLength)
            break;
        accumulatedLength += segLength;
    }

    // Calculate interpolation factor between the segment points
    float segLength = segmentLengths[segmentIndex];
    float t = (targetLength - accumulatedLength) / segLength;

    // Retrieve points for interpolation, wrapping around if necessary
    const auto& p0 = points[segmentIndex];
    const auto& p1 = points[(segmentIndex + 1) % N];

    // Interpolate frequency logarithmically
    float logFreq0 = log10f(p0.first);
    float logFreq1 = log10f(p1.first);
    float logFreq = logFreq0 + t * (logFreq1 - logFreq0);
    float frequency = powf(10.0f, logFreq);

    // Interpolate pan linearly
    float pan = p0.second + t * (p1.second - p0.second);

    return { frequency, pan };
}
