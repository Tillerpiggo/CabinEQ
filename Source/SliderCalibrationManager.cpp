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
    
    for (int i = 0; i < numPoints; ++i)
    {
        randomIndices.push_back (i);
    }
    
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle (randomIndices.begin(), randomIndices.end(), rd);
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
    
    sliderSequencer.playInterval (sliderSetPointManager.getFrequencyAt (randomIndices[currBlindIdx]),
                                  sliderSetPointManager.getAmplitudeAt (randomIndices[currBlindIdx]),
                                  sliderSetPointManager.getPanAt (randomIndices[currBlindIdx]),
                                  noteLength,
                                  false);
    
    return currBlindIdx;
}

void SliderCalibrationManager::updateReferenceTone()
{
    sliderSequencer.setReferenceNote (sliderSetPointManager.getFrequencyAt (referenceToneIdx),
                                      sliderSetPointManager.getAmplitudeAt (referenceToneIdx));
}

//===================================
// Testing

// Getters
bool SliderCalibrationManager::getIsTesting() const {
    return isTesting;
}

bool SliderCalibrationManager::getIsFilterEnabled() const {
    return isFilterEnabled;
}

float SliderCalibrationManager::getFilterGain() const {
    if (isFilterEnabled) return juce::Decibels::decibelsToGain (filterGain);
    else return 1.0f; // don't change gain if filter isn't enabled
}

// Setters
void SliderCalibrationManager::setIsTesting (bool isTesting) {
    this->isTesting = isTesting;
    sliderSequencer.playTestingInterval(tone1Freq, tone1Vol,
                                        tone2Freq, tone2Vol);
}

void SliderCalibrationManager::setIsFilterEnabled (bool isFilterEnabled) {
    this->isFilterEnabled = isFilterEnabled;
    std::cout << "Filter enabled: " << isFilterEnabled << std::endl;
}

void SliderCalibrationManager::setTone1Freq (float tone1Freq) {
    this->tone1Freq = tone1Freq;
    sliderSequencer.playTestingInterval (tone1Freq, tone1Vol,
                                        tone2Freq, tone2Vol);
}

void SliderCalibrationManager::setTone2Freq (float tone2Freq) {
    this->tone2Freq = tone2Freq;
    sliderSequencer.playTestingInterval (tone1Freq, tone1Vol,
                                        tone2Freq, tone2Vol);
}

void SliderCalibrationManager::setTone1Vol (float tone1Vol) {
    this->tone1Vol = tone1Vol;
    sliderSequencer.playTestingInterval (tone1Freq, tone1Vol,
                                        tone2Freq, tone2Vol);
}

void SliderCalibrationManager::setTone2Vol (float tone2Vol) {
    this->tone2Vol = tone2Vol;
    sliderSequencer.playTestingInterval (tone1Freq, tone1Vol,
                                        tone2Freq, tone2Vol);
}

void SliderCalibrationManager::setFilterGain (float filterGain) {
    this->filterGain = filterGain;
    std::cout << "set filter gain to: " << filterGain << std::endl;
}
