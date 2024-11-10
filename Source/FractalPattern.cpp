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
    // The order of the Moore curve is based on the complexity
    order = complexity;
    size = 1 << order; // size = 2^order
    numPoints = size * size; // Total number of points in the curve

    if (complexity <= 0)
        throw std::invalid_argument("Complexity must be a positive integer.");

    // Precompute the Moore curve points
    generateMooreCurve();
}

void FractalPattern::generateMooreCurve()
{
    points.clear();
    int x = 0, y = 0;

    // Generate the Moore curve recursively
    mooreCurve(order, 0, x, y);
}

// Direction encoding: 0 - Up, 1 - Right, 2 - Down, 3 - Left
void FractalPattern::mooreCurve(int level, int dir, int &x, int &y)
{
    if (level == 0)
    {
        // Add point to the curve
        float x_norm = x / static_cast<float>(size - 1);
        float y_norm = y / static_cast<float>(size - 1);

        // Map x_norm to frequency (logarithmic scale)
        float f_min = 20.0f;      // Minimum frequency (Hz)
        float f_max = 20000.0f;   // Maximum frequency (Hz)
        float log_f_min = std::log10(f_min);
        float log_f_max = std::log10(f_max);

        float log_f = x_norm * (log_f_max - log_f_min) + log_f_min;
        float frequency = std::pow(10.0f, log_f);

        // Map y_norm to pan [-1, 1]
        float pan = y_norm * 2.0f - 1.0f;

        points.emplace_back(frequency, pan);
        return;
    }

    switch (dir)
    {
    case 0:
        mooreCurve(level - 1, 1, x, y);
        y++;
        mooreCurve(level - 1, 0, x, y);
        x++;
        mooreCurve(level - 1, 0, x, y);
        y--;
        mooreCurve(level - 1, 3, x, y);
        break;
    case 1:
        mooreCurve(level - 1, 0, x, y);
        x++;
        mooreCurve(level - 1, 1, x, y);
        y++;
        mooreCurve(level - 1, 1, x, y);
        x--;
        mooreCurve(level - 1, 2, x, y);
        break;
    case 2:
        mooreCurve(level - 1, 3, x, y);
        y--;
        mooreCurve(level - 1, 2, x, y);
        x--;
        mooreCurve(level - 1, 2, x, y);
        y++;
        mooreCurve(level - 1, 1, x, y);
        break;
    case 3:
        mooreCurve(level - 1, 2, x, y);
        x--;
        mooreCurve(level - 1, 3, x, y);
        y--;
        mooreCurve(level - 1, 3, x, y);
        x++;
        mooreCurve(level - 1, 0, x, y);
        break;
    }
}

std::pair<float, float> FractalPattern::getFrequencyAndPanAtTime(float time)
{
    // Normalize time to [0, 1)
    float t_mod = fmod(time, 1.0f);
    if (t_mod < 0.0f)
        t_mod += 1.0f;

    // Calculate the index into the precomputed points
    int index = static_cast<int>(t_mod * points.size()) % points.size();

    return points[index];
}
