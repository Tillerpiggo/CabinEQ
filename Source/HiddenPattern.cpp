/*
  ==============================================================================

    HiddenPattern.cpp
    Created: 7 Nov 2024 11:09:23am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "HiddenPattern.h"

HiddenPattern::HiddenPattern (std::vector<bool> hits, std::vector<std::pair<float, float>> hiddenPath, std::vector<std::pair<float, float>> confoundingCoords, float hiddenBandwidth, float confoundingBandwidth, float cycleLengthInSeconds)
    : hits (hits), hiddenPath (hiddenPath), confoundingCoords (confoundingCoords), cycleLengthInSeconds (cycleLengthInSeconds), hiddenBandwidth (hiddenBandwidth), confoundingBandwidth (confoundingBandwidth)
{}

HiddenPattern::HiddenPattern(std::vector<bool> hits,
                             std::vector<std::vector<int>> pattern2D,
                             float hiddenBandwidth,
                             float confoundingBandwidth,
                             float cycleLengthInSeconds)
    : HiddenPattern(hits,
                    GenerateHiddenPath(pattern2D),
                    GenerateConfoundingCoords(pattern2D),
                    hiddenBandwidth,
                    confoundingBandwidth,
                    cycleLengthInSeconds) {}

//HiddenPattern HiddenPattern::alongPath(std::vector<std::pair<float, float>> path, int numHits)
//{
//    // Total number of points, including confounding points
//    int totalNumPoints = (2 * numHits) + 1; // For pattern like 0|1|0|1|...|0
//
//    // Generate hits vector: 0|1|0|1|...|0
//    std::vector<bool> hits(totalNumPoints);
//    for (int i = 0; i < totalNumPoints; ++i)
//    {
//        hits[i] = (i % 2 == 0) ? false : true; // Even indices are 0 (confounding), odd are 1 (hidden)
//    }
//
//    // Generate evenly spaced parameter values along the path
//    std::vector<float> t_values(totalNumPoints);
//    for (int i = 0; i < totalNumPoints; ++i)
//    {
//        t_values[i] = static_cast<float>(i) / static_cast<float>(totalNumPoints - 1);
//    }
//
//    // Generate positions along the path at parameter values t_values
//    std::vector<std::pair<float, float>> positions(totalNumPoints);
//    for (int i = 0; i < totalNumPoints; ++i)
//    {
//        positions[i] = GetPositionAlongPath(path, t_values[i]);
//    }
//
//    // Separate positions into hiddenPath and confoundingCoords
//    std::vector<std::pair<float, float>> hiddenPath;
//    std::vector<std::pair<float, float>> confoundingCoords;
//
//    // Shift confounding points perpendicularly off the path
//    for (int i = 0; i < totalNumPoints; ++i)
//    {
//        if (hits[i])
//        {
//            // Hidden points remain on the path
//            hiddenPath.push_back(positions[i]);
//        }
//        else
//        {
//            // Confounding points: shift perpendicularly from the path
//            std::pair<float, float> dir = GetDirectionAlongPath(path, t_values[i]);
//
//            // Perpendicular direction (-dy, dx)
//            std::pair<float, float> perpDir = std::make_pair(-dir.second, dir.first);
//
//            // Normalize perpendicular direction
//            float length = std::sqrt(perpDir.first * perpDir.first + perpDir.second * perpDir.second);
//            perpDir.first /= length;
//            perpDir.second /= length;
//
//            // Shift position by a fixed offset
//            float offset = 0.1f; // Adjust offset as needed
//            std::pair<float, float> confPos = std::make_pair(
//                positions[i].first + perpDir.first * offset,
//                positions[i].second + perpDir.second * offset
//            );
//
//            confoundingCoords.push_back(confPos);
//        }
//    }
//
//    // Set example values for bandwidths and cycle length
//    float hiddenBandwidth = 0.5f;          // Adjust as needed
//    float confoundingBandwidth = 0.5f;     // Adjust as needed
//    float cycleLengthInSeconds = 1.0f;     // Adjust as needed
//
//    // Create the HiddenPattern object
//    HiddenPattern hiddenPattern({ 1, 0, 1, 0 }, hiddenPath, confoundingCoords, hiddenBandwidth, confoundingBandwidth, cycleLengthInSeconds);
//
//    return hiddenPattern;
//}

HiddenPattern HiddenPattern::alongPath(std::vector<std::pair<float, float>> path, int numHits)
{
    // Total number of points, including confounding points
    int totalNumPoints = (2 * numHits) + 1; // For pattern like 0|1|0|1|...|0

    // Generate hits vector: 0|1|0|1|...|0
    std::vector<bool> hits(totalNumPoints);
    for (int i = 0; i < totalNumPoints; ++i)
    {
        hits[i] = (i % 2 == 0) ? false : true; // Even indices are 0 (confounding), odd are 1 (hidden)
    }

    // Generate evenly spaced parameter values along the path
    std::vector<float> t_values(totalNumPoints);
    for (int i = 0; i < totalNumPoints; ++i)
    {
        t_values[i] = static_cast<float>(i) / static_cast<float>(totalNumPoints - 1);
    }

    // Generate positions along the path at parameter values t_values
    std::vector<std::pair<float, float>> positions(totalNumPoints);
    for (int i = 0; i < totalNumPoints; ++i)
    {
        positions[i] = GetPositionAlongPath(path, t_values[i]);
    }

    // Separate positions into hiddenPath and confoundingCoords
    std::vector<std::pair<float, float>> hiddenPath;
    std::vector<std::pair<float, float>> confoundingCoords;

    // Shift confounding points perpendicularly off the path
    for (int i = 0; i < totalNumPoints; ++i)
    {
        // Compute direction at t_values[i]
        std::pair<float, float> dir = GetDirectionAlongPath(path, t_values[i]);

        // Perpendicular direction (-dy, dx)
        std::pair<float, float> perpDir = std::make_pair(-dir.second, dir.first);

        // Normalize perpendicular direction
        float length = std::sqrt(perpDir.first * perpDir.first + perpDir.second * perpDir.second);
        perpDir.first /= length;
        perpDir.second /= length;

        // Shift position by a fixed offset
        float offset = 0.1f; // Adjust offset as needed

        if (hits[i])
        {
            // Hidden points remain on the path
            hiddenPath.push_back(positions[i]);

            // **Add confounding points that surround the hidden points orthogonally**

            // Generate confounding points on both sides of the hidden point
            // First side
            std::pair<float, float> confPos1 = std::make_pair(
                positions[i].first + perpDir.first * offset,
                positions[i].second + perpDir.second * offset
            );

            // Second side
            std::pair<float, float> confPos2 = std::make_pair(
                positions[i].first - perpDir.first * offset,
                positions[i].second - perpDir.second * offset
            );

//            confoundingCoords.push_back(confPos1);
//            confoundingCoords.push_back(confPos2);
        }
        else
        {
            // Existing confounding points: shift perpendicularly from the path

            std::pair<float, float> confPos = std::make_pair(
                positions[i].first,// + perpDir.first * offset,
                positions[i].second// + perpDir.second * offset
            );

            confoundingCoords.push_back(confPos);
        }
    }

    // Set example values for bandwidths and cycle length
    float hiddenBandwidth = 0.25f;          // Adjust as needed
    float confoundingBandwidth = 0.25f;     // Adjust as needed
    float cycleLengthInSeconds = 1.0f;     // Adjust as needed

    // **Use the generated hits vector instead of hardcoded values**
    HiddenPattern hiddenPattern(hits, hiddenPath, confoundingCoords, hiddenBandwidth, confoundingBandwidth, cycleLengthInSeconds);

    return hiddenPattern;
}

// Helper function to compute position along the path at parameter t in [0,1]
std::pair<float, float> HiddenPattern::GetPositionAlongPath(const std::vector<std::pair<float, float>>& path, float t)
{
    // Calculate total length of the path
    std::vector<float> segmentLengths;
    float totalLength = 0.0f;

    for (size_t i = 0; i < path.size() - 1; ++i)
    {
        float dx = path[i + 1].first - path[i].first;
        float dy = path[i + 1].second - path[i].second;
        float length = std::sqrt(dx * dx + dy * dy);
        segmentLengths.push_back(length);
        totalLength += length;
    }

    // Calculate the target distance along the path
    float targetLength = t * totalLength;

    // Find the segment where the target length falls
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < segmentLengths.size(); ++i)
    {
        if (accumulatedLength + segmentLengths[i] >= targetLength)
        {
            float segmentT = (targetLength - accumulatedLength) / segmentLengths[i];
            float x = path[i].first + segmentT * (path[i + 1].first - path[i].first);
            float y = path[i].second + segmentT * (path[i + 1].second - path[i].second);
            return std::make_pair(x, y);
        }
        accumulatedLength += segmentLengths[i];
    }

    // If t == 1, return the last point
    return path.back();
}

// Helper function to compute direction along the path at parameter t in [0,1]
std::pair<float, float> HiddenPattern::GetDirectionAlongPath(const std::vector<std::pair<float, float>>& path, float t)
{
    // Calculate total length of the path
    std::vector<float> segmentLengths;
    float totalLength = 0.0f;

    for (size_t i = 0; i < path.size() - 1; ++i)
    {
        float dx = path[i + 1].first - path[i].first;
        float dy = path[i + 1].second - path[i].second;
        float length = std::sqrt(dx * dx + dy * dy);
        segmentLengths.push_back(length);
        totalLength += length;
    }

    // Calculate the target distance along the path
    float targetLength = t * totalLength;

    // Find the segment where the target length falls
    float accumulatedLength = 0.0f;
    for (size_t i = 0; i < segmentLengths.size(); ++i)
    {
        if (accumulatedLength + segmentLengths[i] >= targetLength)
        {
            // Direction is from path[i] to path[i + 1]
            float dx = path[i + 1].first - path[i].first;
            float dy = path[i + 1].second - path[i].second;
            // Normalize direction
            float length = std::sqrt(dx * dx + dy * dy);
            dx /= length;
            dy /= length;
            return std::make_pair(dx, dy);
        }
        accumulatedLength += segmentLengths[i];
    }

    // If t == 1, return the direction of the last segment
    size_t i = path.size() - 2;
    float dx = path[i + 1].first - path[i].first;
    float dy = path[i + 1].second - path[i].second;
    float length = std::sqrt(dx * dx + dy * dy);
    dx /= length;
    dy /= length;
    return std::make_pair(dx, dy);
}

// Helper functions to generate hiddenPath and confoundingCoords
std::vector<std::pair<float, float>> HiddenPattern::GenerateHiddenPath(const std::vector<std::vector<int>>& pattern2D)
{
    std::vector<std::pair<int, std::pair<float, float>>> orderedPoints;
    int numRows = pattern2D.size();
    int numCols = pattern2D.empty() ? 0 : pattern2D[0].size();

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int value = pattern2D[r][c];
            if (value > 0) {
                float x = (numCols == 1) ? 0.0f : -1.0f + 2.0f * c / (numCols - 1);
                float y = (numRows == 1) ? 0.0f : -1.0f + 2.0f * r / (numRows - 1);
                orderedPoints.emplace_back(value, std::make_pair(x, y));
            }
        }
    }
    std::sort(orderedPoints.begin(), orderedPoints.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    std::vector<std::pair<float, float>> hiddenPath;
    for (const auto& p : orderedPoints) {
        hiddenPath.push_back(p.second);
    }
    return hiddenPath;
}

std::vector<std::pair<float, float>> HiddenPattern::GenerateConfoundingCoords(const std::vector<std::vector<int>>& pattern2D)
{
    std::vector<std::pair<float, float>> confoundingCoords;
    int numRows = pattern2D.size();
    int numCols = pattern2D.empty() ? 0 : pattern2D[0].size();

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int value = pattern2D[r][c];
            if (value == 0) {
                float x = (numCols == 1) ? 0.0f : -1.0f + 2.0f * c / (numCols - 1);
                float y = (numRows == 1) ? 0.0f : -1.0f + 2.0f * r / (numRows - 1);
                confoundingCoords.emplace_back(x, y);
            }
        }
    }
    return confoundingCoords;
}

SweepPattern HiddenPattern::getHiddenSweepPattern (float sampleRate)
{
    std::vector<std::pair<float, float>> hiddenPathFreqsAndPans;
    for (const auto& point : hiddenPath)
    {
        hiddenPathFreqsAndPans.push_back (frequencyAndPanForCoords (point));
    }
    return SweepPattern (hiddenPathFreqsAndPans, cycleLengthInSeconds, sampleRate);
}

MelodicNotes HiddenPattern::getMelodicPattern()
{
    std::vector<float> freqs;
    std::vector<float> pans;
    for (const auto& point : hiddenPath)
    {
        auto [freq, pan] = frequencyAndPanForCoords (point);
        freqs.push_back (freq);
        pans.push_back (pan);
    }
    
    return MelodicNotes::withMelodicPattern (hits, freqs, hiddenBandwidth, pans).withNoteDurationInSeconds (0.1f);
}

std::vector<SweepPattern> HiddenPattern::getConfoundingSweepPatterns (float sampleRate)
{
    std::vector<SweepPattern> confoundingSweepPatterns;
    for (int i = 0; i < confoundingCoords.size(); ++i)
    {
        auto coords = frequencyAndPanForCoords (confoundingCoords[i]);
        confoundingSweepPatterns.push_back (SweepPattern ({ coords, coords }, cycleLengthInSeconds, sampleRate));
    }
    
    return confoundingSweepPatterns;
}

float HiddenPattern::getHiddenBandwidth()
{
    return hiddenBandwidth;
}

float HiddenPattern::getConfoundingBandwidth()
{
    return confoundingBandwidth;
}

std::pair<float, float> HiddenPattern::frequencyAndPanForCoords (std::pair<float, float> coords)
{
    auto [x, y] = coords;
    
    float freq = MIN_FREQ * std::pow (MAX_FREQ / MIN_FREQ, (y + 1.0f) / 2.0f );
    float pan = x;

    return { freq, pan };
}
