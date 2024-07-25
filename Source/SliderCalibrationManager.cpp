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
    return setPointManager.indexForFrequency (sliderSequencer.currentlyPlayingFrequency());
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
    sliderSequencer.playInterval (setPointManager.getSetPoints().at (idx),
                                  noteLength,
                                  false);
}

void SliderCalibrationManager::setAmplitudeAtIdx (int idx, float newAmplitude)
{
    setPointManager.setAmplitudeAt (idx, newAmplitude);
    sliderSequencer.changeAmplitudeOfNotesWithFrequency (setPointManager.getSetPoints().at (idx).frequency,
                                                         newAmplitude);
}

void SliderCalibrationManager::setPanAtIdx (int idx, float newPan)
{
    setPointManager.setPanAt (idx, newPan);
    sliderSequencer.changePanOfNotesWithFrequency (setPointManager.getSetPoints().at (idx).pan,
                                                   newPan);
}

void SliderCalibrationManager::setIsCalibrating (bool isCalibrating)
{
    this->isCalibrating = isCalibrating;
}

void SliderCalibrationManager::updateCurve()
{
    curve.updateWithSetPoints (setPointManager.getSetPoints());
}

void SliderCalibrationManager::changeNoteLength (int newNoteLength)
{
    noteLength = newNoteLength;
    if (currIdx != -1) setCurrIdx (currIdx);
}
