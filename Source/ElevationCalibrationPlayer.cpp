/*
  ==============================================================================

    ElevationCalibrationPlayer.cpp
    Created: 23 Feb 2025 10:25:46am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ElevationCalibrationPlayer.h"

ElevationCalibrationPlayer::ElevationCalibrationPlayer()
    : tiltFilter (14)
{
    
}

std::pair<float, float> ElevationCalibrationPlayer::getNextSample()
{
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    for (int i = 0; i < rowPlayers.size(); ++i)
    {
        auto nextSample = rowPlayers[i].getNextSample();
        leftSample += nextSample.first;
        rightSample += nextSample.second;
    }

    return { leftSample, rightSample };
}


void ElevationCalibrationPlayer::processBlock(juce::AudioBuffer<float>& buffer, float gain)
{
    updateRowPlayersIfNeeded();

    // Make copy of buffer
    juce::AudioBuffer<float> copyBuffer;
    copyBuffer.makeCopyOf (buffer);
    copyBuffer.clear();

    // Populate buffer
    auto* leftChannel = copyBuffer.getWritePointer (0);
    auto* rightChannel = copyBuffer.getNumChannels() > 1 ? copyBuffer.getWritePointer (1) : nullptr;

    for (int sample = 0; sample < copyBuffer.getNumSamples(); ++sample)
    {
        auto nextSample = getNextSample();
        leftChannel[sample] += nextSample.first * 0.15 * 0.5 * gain;

        if (rightChannel)
            rightChannel[sample] += nextSample.second * 0.15 * 0.5 * gain;
    }
}

void ElevationCalibrationPlayer::prepare(const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve, 14);
}

void ElevationCalibrationPlayer::setCalibration(ElevationCalibration calibration)
{
    this->calibration = calibration;
    shouldUpdateRowPlayers = true;
}

void ElevationCalibrationPlayer::setBandwidth(float bandwidthOctaves)
{
    this->bandwidth = bandwidthOctaves;
    shouldUpdateRowPlayers = true;
}

std::vector<float> ElevationCalibrationPlayer::getCurrPlayingFreqs()
{
    std::vector<float> freqs;
    for (int i = 0; i < rowPlayers.size(); ++i)
    {
        freqs.push_back (getFrequencyForRow (i));
    }
    return freqs;
}

void ElevationCalibrationPlayer::updateRowPlayersIfNeeded()
{
    if (!shouldUpdateRowPlayers)
        return;

    rowPlayers.clear();

    auto numRows = calibration.getNumRows();
    auto selectedRow = calibration.getSelectedRow();

    for (int row = 0; row < numRows; ++row)
    {
        auto freq = getFrequencyForRow(row);
        rowPlayers.push_back (RowPlayer());
        rowPlayers.back().prepare (spec);
        rowPlayers.back().setFrequency (freq);
        rowPlayers.back().setBandwidth (bandwidth);
    }

    shouldUpdateRowPlayers = false;
}
float ElevationCalibrationPlayer::getFrequencyForRow(int row) const
{
    auto numRows = calibration.getNumRows();

    // Evenly subdivide numRows-1 divisions between MIN_FREQ and MAX_FREQ
    float numOctaves = std::log2(MAX_FREQ / MIN_FREQ);
    float octavePerBand = numOctaves / static_cast<float> (numRows - 1);

    // Return logarithmically spaced frequency for this row
    return MIN_FREQ * std::pow(2.0f, octavePerBand * row);
}

