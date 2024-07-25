/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderCalibrationManager.h"
#include <cmath>
#include <random>

SliderCalibrationManager::SliderCalibrationManager()
    : isCalibrating (false)
{
    int numPoints = sliderSetPointManager.getNumPoints();
    updateCurve();
}

const Curve& SliderCalibrationManager::getCurve()
{
    updateCurve();
    return curve;
}

std::pair<float, float> SliderCalibrationManager::getNextSample()
{
    return sliderSequencer.getNextSample();
}

int SliderCalibrationManager::getCurrentlyPlayingIdx()
{
    return sliderSetPointManager.indexForFrequency (sliderSequencer.currentlyPlayingFrequency());
}

bool SliderCalibrationManager::getIsCalibrating() const
{
    return isCalibrating;
}

void SliderCalibrationManager::setSampleRate (float newSampleRate)
{
    sliderSequencer.setSampleRate (newSampleRate);
}

void SliderCalibrationManager::setCurrIdx (int idx)
{
    currIdx = idx;
    sliderSequencer.playInterval (sliderSetPointManager.getFrequencyAt (idx),
                                  sliderSetPointManager.getAmplitudeAt (idx),
                                  sliderSetPointManager.getPanAt (idx),
                                  noteLength,
                                  false);
    
    std::cout << "Freq: " << sliderSetPointManager.getFrequencyAt (idx) << std::endl;
}

void SliderCalibrationManager::setAmplitudeAtIdx (int idx, float newAmplitude)
{
    sliderSetPointManager.setAmplitudeAt (idx, newAmplitude);
    sliderSequencer.changeAmplitudeOfNotesWithFrequency (sliderSetPointManager.getFrequencyAt (idx),
                                                         newAmplitude);
}

void SliderCalibrationManager::setPanAtIdx (int idx, float newPan)
{
    sliderSetPointManager.setPanAt (idx, newPan);
    sliderSequencer.changePanOfNotesWithFrequency (sliderSetPointManager.getFrequencyAt (idx),
                                                   newPan);
}

void SliderCalibrationManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void SliderCalibrationManager::updateCurve()
{
    curve.setFrequencies (sliderSetPointManager.getFrequencies());
    std::vector<float> amplitudePoints = sliderSetPointManager.getAmplitudes();
    std::vector<float> panPoints = sliderSetPointManager.getPans();
    std::vector<float> frequencies = sliderSetPointManager.getFrequencies();
    
    curve.setAmplitudes (amplitudePoints);
    curve.setPans (panPoints);
    curve.setPhases (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
}

void SliderCalibrationManager::changeNoteLength (int newNoteLength)
{
    noteLength = newNoteLength;
    if (currIdx != -1) setCurrIdx (currIdx);
}
