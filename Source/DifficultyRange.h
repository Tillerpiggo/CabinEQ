/*
  ==============================================================================

    DifficultyRange.h
    Created: 30 Oct 2024 1:31:32pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

// This maps a value from 0 to 1 differing tempo, noise volume, and other confounding factors that can make a pattern more difficult to hear
class DifficultyRange
{
public:
    DifficultyRange (float lowestSpeedFactor = 1.0f, float highestSpeedFactor = 1.0f, float lowestNoiseLevelPercent = 1.0f, float highestNoiseLevelPercent = 1.0f)
        : lowestSpeedFactor (lowestSpeedFactor), highestSpeedFactor (highestSpeedFactor),
          lowestNoiseLevelPercent (lowestNoiseLevelPercent), highestNoiseLevelPercent (highestNoiseLevelPercent)
    {}
    
    std::pair<float, float> getSpeedAndNoiseLevelForDifficulty (float difficulty) // difficulty from 0 (easiest) to 1 (hardest)
    {
        if (difficulty < 0.0f || difficulty > 1.0f)
            std::cerr << "Trying to call getSpeedAndNoiseLevelForDifficulty for a difficulty outside of [0, 1], which is undefined" << std::endl;
        
        // Linearly interpolate speed and noise level
        float speedFactor = lowestSpeedFactor + (highestSpeedFactor - lowestSpeedFactor) * difficulty;
        float noiseLevelPercent = lowestNoiseLevelPercent + (highestNoiseLevelPercent - lowestNoiseLevelPercent) * difficulty;
        
        return { speedFactor, noiseLevelPercent };
    }
    
private:
    float lowestSpeedFactor;
    float highestSpeedFactor;
    float lowestNoiseLevelPercent;
    float highestNoiseLevelPercent;
};
