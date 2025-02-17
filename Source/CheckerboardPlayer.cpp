/*
  ==============================================================================

    CheckerboardPlayer.cpp
    Created: 10 Feb 2025 3:18:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CheckerboardPlayer.h"

CheckerboardPlayer::CheckerboardPlayer()
    : tiltFilter (14)
{
    systemVolumeProcessor.setRampDurationSeconds (0.05);
    systemVolumeProcessor.setGainLinear (juce::SystemAudioVolume::getGain());
}

void CheckerboardPlayer::processBlock (juce::AudioBuffer<float>& buffer, float gain)
{
    // Volume processing
    systemVolumeProcessor.setGainLinear (juce::SystemAudioVolume::getGain());
    
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
    
    // Filter buffer
    juce::dsp::AudioBlock<float> copyBlock (copyBuffer);
    juce::dsp::ProcessContextReplacing<float> copyContext (copyBlock);
    tiltFilter.process (copyContext);
    systemVolumeProcessor.process (copyContext);
    
    // Add copy buffer contents back to main buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.addFrom (channel, 0, copyBuffer, channel, 0, copyBuffer.getNumSamples());
    }
}

std::pair<float, float> CheckerboardPlayer::getNextSample()
{
    updateNoiseGeneratorsIfNeeded();
    
    // Compute next sample
    float leftSample = 0.0f;
    float rightSample = 0.0f;
    for (int i = 0; i < noiseGenerators.size(); ++i)
    {
        auto noiseSample = noiseGenerators[i].getNextSample();
        leftSample += noiseSample.first;
        rightSample += noiseSample.second;
    }
    
    return { leftSample, rightSample };
}

void CheckerboardPlayer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve, 14);
}

void CheckerboardPlayer::setCheckerboard (Checkerboard checkerboard)
{
    this->checkerboard = checkerboard;
    shouldUpdateNoiseGenerators = true;
}

void CheckerboardPlayer::setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords)
{
    this->soloSquareCoords = soloSquareCoords;
    std::cout << "set solo square coords" << std::endl;
    shouldUpdateNoiseGenerators = true;
}

std::vector<float> CheckerboardPlayer::getCurrSolodFreqs()
{
    // Just return the solod freqs without thinking too hard about it
    std::set<float> solodFreqs;
//    for (i)
}

float CheckerboardPlayer::getBandwidth()
{
    // TODO: implement
    return 1.0f;
}

void CheckerboardPlayer::updateNoiseGeneratorsIfNeeded()
{
    if (! shouldUpdateNoiseGenerators)
        return;
    
    noiseGenerators.clear();
    
//    int resolution = checkerboard.getResolution();
    auto [numRows, numCols] = checkerboard.getGridDimensions();
//    float sharpness = checkerboard.getSharpness();
    
//    // Clear noise generators and add new ones
//    if (! soloSquareCoords.empty()) // if solo'd, just play the one square
//    {
//        int lastIdx = 0;
//        for (const auto& soloSquareCoordPair : soloSquareCoords)
//        {
//            noiseGenerators.push_back (SquareGenerator());
//            noiseGenerators[lastIdx].prepare (spec);
//            noiseGenerators[lastIdx].setResolution (resolution);
//            noiseGenerators[lastIdx].setSharpness (sharpness);
//            noiseGenerators[lastIdx].setCheckerboardCoords (soloSquareCoordPair.first, soloSquareCoordPair.second);
//            lastIdx++;
//        }
//    }
//    else 
//    {
        auto grid = checkerboard.getGrid();
        int lastIdx = 0;
        for (int freqIdx = 0; freqIdx < numRows; ++freqIdx)
        {
            for (int panIdx = 0; panIdx < numCols; ++panIdx)
            {
                bool isSoloed = soloSquareCoords.empty() || (soloSquareCoords.find({ freqIdx, panIdx }) != soloSquareCoords.end());
                if (grid[freqIdx][panIdx] && isSoloed)
                {
                    noiseGenerators.push_back (SquareGenerator());
                    noiseGenerators[lastIdx].prepare (spec);
                    noiseGenerators[lastIdx].setGridDimensions (numRows, numCols);
                    noiseGenerators[lastIdx].setCheckerboardCoords (freqIdx, panIdx);
                    lastIdx++;
                }
            }
        }
//    }

    
    shouldUpdateNoiseGenerators = false;
}
