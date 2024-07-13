/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderCalibrationManager.h"

SliderCalibrationManager::SliderCalibrationManager()
    : isCalibrating (false)
{
    int numPoints = sliderSetPointManager.getNumPoints();

    tuningIndices = { 4, 5, 6, 7 };
    
    for (int i = 0; i < numPoints; ++i) tuningIndexCounts[i] = 0;
    for (const auto& idx : tuningIndices) tuningIndexCounts[idx]++;
    
    playTuningNotes();
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

int SliderCalibrationManager::getCurrentlyPlayingTuningIdx()
{
    int idx = getCurrentlyPlayingIdx();
    for (int i = 0; i < tuningIndices.size(); ++i)
    {
        if (tuningIndices[i] == idx)
        {
            return i;
        }
    }
    
    return -1;
}

bool SliderCalibrationManager::getIsCalibrating() const
{
    return isCalibrating;
}

const std::vector<float>& SliderCalibrationManager::getTuningIndices()
{
    return tuningIndices;
}

void SliderCalibrationManager::setSampleRate (float newSampleRate)
{
    sliderSequencer.setSampleRate (newSampleRate);
}

void SliderCalibrationManager::setCurrIdx (int idx)
{
    sliderSequencer.playInterval (sliderSetPointManager.getFrequencyAt (idx),
                                  sliderSetPointManager.getAmplitudeAt (idx),
                                  sliderSetPointManager.getPanAt (idx));
//    if (idx == 0 || idx == 8)
//    {
//        sliderSequencer.playInterval (sliderSetPointManager.getFrequencyAt (idx),
//                                      sliderSetPointManager.getAmplitudeAt (idx),
//                                      sliderSetPointManager.getPanAt (idx));
//    }
//    else
//    {
//        sliderSequencer.playIntervalAndLastNote (sliderSetPointManager.getFrequencyAt (idx), 
//                                                 sliderSetPointManager.getAmplitudeAt (idx),
//                                                 sliderSetPointManager.getPanAt (idx),
//                                                 sliderSetPointManager.getFrequencyAt (idx - 1),
//                                                 sliderSetPointManager.getAmplitudeAt (idx - 1),
//                                                 sliderSetPointManager.getPanAt (idx - 1));
//    }
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

void SliderCalibrationManager::changeTuningIndices() {
    std::cout << "changing tuning indices" << std::endl;
    std::vector<float> frequencies = sliderSetPointManager.getFrequencies();
    int numFrequencies = frequencies.size();

    // Find the minimum play count
    int minCount = std::numeric_limits<int>::max();
    for (const auto& [index, count] : tuningIndexCounts) {
        if (count < minCount) {
            minCount = count;
        }
    }
    
    std::cout << "minCount: " << minCount << std::endl;

    // Collect indices with the minimum play count
    std::vector<int> minCountIndices;
    for (const auto& [index, count] : tuningIndexCounts) {
        if (count == minCount) {
            minCountIndices.push_back(index);
            std::cout << "adding index: " << index << std::endl;
        }
    }

    // Filter available indices that are not in tuningIndices and have the minimum count
    std::vector<int> availableIndices;
    std::copy_if(minCountIndices.begin(), minCountIndices.end(), std::back_inserter(availableIndices),
                 [this](int index) {
                     return std::find(tuningIndices.begin(), tuningIndices.end(), index) == tuningIndices.end();
                 });

    if (availableIndices.empty()) {
        // No available indices to substitute
        return;
    }

    // Choose a random index from the available indices
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> distAvail(0, static_cast<int>(availableIndices.size()) - 1);
    int newIndex = availableIndices[distAvail(rng)];

    // Choose a random index in tuningIndices to substitute
    std::uniform_int_distribution<> distTuning(0, static_cast<int>(tuningIndices.size()) - 1);
    int indexToReplace = distTuning(rng);

    tuningIndices[indexToReplace] = newIndex;
    tuningIndexCounts[newIndex]++;
    
    playTuningNotes();
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

void SliderCalibrationManager::playTuningNotes()
{
    std::vector<float> allFrequencies = sliderSetPointManager.getFrequencies();
    std::vector<float> allAmplitudes = sliderSetPointManager.getAmplitudes();
    std::vector<float> allPans = sliderSetPointManager.getPans();
    std::vector<float> subsetFrequencies;
    std::vector<float> subsetAmplitudes;
    std::vector<float> subsetPans;
    
    for (const auto& idx : tuningIndices) {
        subsetFrequencies.push_back(allFrequencies[idx]);
        subsetAmplitudes.push_back(allAmplitudes[idx]);
        subsetPans.push_back(allPans[idx]);
    }
    
    sliderSequencer.playTuningNotes (subsetFrequencies, subsetAmplitudes, subsetPans);
    //sliderSequencer.playRandomNotes (subsetFrequencies, subsetAmplitudes, subsetPans);
    std::cout << "Playing Tuning Notes" << std::endl;
}

