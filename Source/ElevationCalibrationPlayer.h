/*
  ==============================================================================

    ElevationCalibrationPlayer.h
    Created: 23 Feb 2025 10:25:46am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RowPlayer.h"
#include "ElevationCalibration.h"
#include "Curve.h"
#include "ArbitraryResponseFilter.h"

class ElevationCalibrationPlayer
{
public:
    ElevationCalibrationPlayer();
    
    std::pair<float, float> getNextSample();
    void processBlock(juce::AudioBuffer<float>& buffer, float gain = 1.0f);
    void prepare(const juce::dsp::ProcessSpec& spec);

    void setElevationCalibration(ElevationCalibration calibration);
    void setBandwidth(float bandwidthOctaves);

    std::vector<float> getCurrPlayingFreqs();
    
private:
    void updateRowGains();
    void updateRowPlayersIfNeeded();
    float getFrequencyForRow(int row) const;

    juce::dsp::ProcessSpec spec;
    ElevationCalibration calibration;
    std::vector<RowPlayer> rowPlayers;
    std::vector<float> rowGains;
    bool shouldUpdateRowPlayers = true;

    ArbitraryResponseFilter tiltFilter;
    TiltCurve tiltCurve;

    float bandwidth = 2.0f;
    float MIN_FREQ = 20.0f;
    float MAX_FREQ = 16000.0f;
    
    float currTime = 0.0f;
    
    juce::Random random;
};


