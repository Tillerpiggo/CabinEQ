/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include "ArbitraryResponseFilter.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : tiltFilter (12),
      isCalibrating (false)
{
    profileVolumeProcessor.setRampDurationSeconds (0.05);
    profileVolumeProcessor.setGainDecibels (0.0f);
    overallVolumeProcessor.setRampDurationSeconds (0.05);
    overallVolumeProcessor.setGainDecibels (0.0f);
    
//    glyphGenerator.setListener (this);
//    melodicNoiseSequencer.setListener (this);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    auto* leftChannel = ioBuffer.getWritePointer(0);
    auto* rightChannel = ioBuffer.getNumChannels() > 1 ? ioBuffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        float volumeOffset = juce::Decibels::decibelsToGain (calibrationVolume);
        for (int sample = 0; sample < ioBuffer.getNumSamples(); ++sample)
        {
            std::pair<float, float> value = getNextSample();
            leftChannel[sample] = value.first * 0.15 * 0.5 * volumeOffset;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.15 * 0.5 * volumeOffset;
        }
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext(ioBlock);
    
    // TODO: combine this audio processing logic for compile-time optimization with processorChain
    if (isCalibrating)
    {
        tiltFilter.process (ioContext);
    }
    
    if (isProcessing)
    {
        filter.process (ioBlock);
        profileVolumeProcessor.process (ioContext);
    }
    overallVolumeProcessor.process (ioContext);
}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    profileVolumeProcessor.setGainDecibels (bandProfile.getVolume());
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    
    glyphGenerator.prepare (spec);
    melodicNoiseSequencer.prepare (spec);
    
    filter.prepare (spec);
    tiltFilter.prepare (spec);
    tiltFilter.updateWithCurve (tiltCurve);
}

void PlaybackManager::setIsProcessing (bool isFilterProcessing)
{
    this->isProcessing = isFilterProcessing;
}

void PlaybackManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void PlaybackManager::setIsCycling (bool isCycling)
{
    this->isCycling = isCycling;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    overallVolumeProcessor.setGainDecibels (volume);
}

float PlaybackManager::getCurrPlayingFreq()
{
    return 1000.0f;
    // TODO: Implement based on QualityStep
}

int PlaybackManager::getStage() const
{
    return stageIdx;
}


void PlaybackManager::setQualityStep (QualityStep qualityStep)
{
    this->qualityStep = qualityStep;
    setStage (0); // set the quality stage to 0 by default
}

int PlaybackManager::getStage()
{
    return stageIdx;
}

void PlaybackManager::setStage (int stageIdx)
{
    this->stageIdx = stageIdx;
    updateSequencersFromQualityStep();
}

//void PlaybackManager::sequenceDidFinish()
//{
//    if (! qualityStep.has_value() || listener == nullptr)
//        return;
//    
//    // Go to the next stage
//    stageIdx++;
//    if (stageIdx >= qualityStep->getNumStages())
//        stageIdx = 0;
//    
//    // Update patterns
//    setStage (stageIdx);
//}

std::pair<float, float> PlaybackManager::getNextSample()
{
    auto [melodicLeftSample, melodicRightSample] = melodicNoiseSequencer.getNextSample();
    auto [glyphLeftSample, glyphRightSample] = glyphGenerator.getNextSample();
    
    return { melodicLeftSample + glyphLeftSample, melodicRightSample + glyphRightSample };
}

void PlaybackManager::updateSequencersFromQualityStep()
{
    if (! qualityStep.has_value())
        std::cerr << "Calling updateSequencersFromQualityStep with null qualityStep in PlaybackManager" << std::endl;
    
    switch (qualityStep->getType())
    {
        case QualityStep::Type::spatial:
        {
            auto glyph = qualityStep->getSpatialPatternAtStage (stageIdx);
            glyphGenerator.setGlyph (glyph);
            melodicNoiseSequencer.mute();
            break;
        }
            
        case QualityStep::Type::intelligibility:
        {
            auto melody = qualityStep->getIntelligibilityPatternAtStage (stageIdx);
            melodicNoiseSequencer.setPattern (melody.noiseNotes());
            glyphGenerator.mute();
            break;
        }
    }
}
