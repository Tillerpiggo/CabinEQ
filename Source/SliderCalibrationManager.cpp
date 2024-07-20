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
//    int comparisonIdx = 0;
//    for (int i = 0; i < comparisonList.size(); ++i)
//    {
//        if (comparisonList[i] == idx)
//        {
//            comparisonIdx = i;
//            break;
//        }
//    }
//    
//    std::vector<float> allFrequencies = sliderSetPointManager.getFrequencies();
//    std::vector<float> allAmplitudes = sliderSetPointManager.getAmplitudes();
//    std::vector<float> allPans = sliderSetPointManager.getPans();
//    std::vector<float> subsetFrequencies;
//    std::vector<float> subsetAmplitudes;
//    std::vector<float> subsetPans;
//    
//    for (int i = 0; i <= comparisonIdx; ++i)
//    {
//        int i2 = comparisonList[i];
//        if (i2 == idx) continue;
//        
//        subsetFrequencies.push_back(allFrequencies[i2]);
//        subsetAmplitudes.push_back(allAmplitudes[i2]);
//        subsetPans.push_back(allPans[i2]);
//    }
//    
//    sliderSequencer.playComparisonFrequencies (sliderSetPointManager.getFrequencyAt (idx),
//                                               sliderSetPointManager.getAmplitudeAt (idx),
//                                               sliderSetPointManager.getPanAt (idx),
//                                               subsetFrequencies, subsetAmplitudes, subsetPans);
}

void SliderCalibrationManager::setAmplitudeAtIdx (int idx, float newAmplitude)
{
    sliderSetPointManager.setAmplitudeAt (idx, newAmplitude);
    sliderSequencer.changeAmplitudeOfNotesWithFrequency (sliderSetPointManager.getFrequencyAt (idx),
                                                         newAmplitude);
    
    if (idx == referenceToneIdx) updateReferenceTone();
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

void SliderCalibrationManager::incrementReferenceToneIndex()
{
    if (referenceToneIdx < SliderSetPointManager::NUM_PTS - 1) referenceToneIdx++;
    updateReferenceTone();
}

void SliderCalibrationManager::decrementReferenceToneIndex()
{
    if (referenceToneIdx > 0) referenceToneIdx--;
    updateReferenceTone();
}

void SliderCalibrationManager::changeNoteLength (int newNoteLength)
{
    noteLength = newNoteLength;
    if (currIdx != -1) setCurrIdx (currIdx);
}

int SliderCalibrationManager::goToNextBlindQuestion()
{
    currBlindIdx++;
    
    if (currBlindIdx > SliderSetPointManager::NUM_PTS) return -1;
    
    return currBlindIdx;
}

void SliderCalibrationManager::updateReferenceTone()
{
    sliderSequencer.setReferenceNote (sliderSetPointManager.getFrequencyAt (referenceToneIdx),
                                      sliderSetPointManager.getAmplitudeAt (referenceToneIdx));
}
