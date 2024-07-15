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

int SliderCalibrationManager::getNumLockedIn()
{
    return static_cast<int> (lockedInIndices.size());
}

bool SliderCalibrationManager::getIsCalibrating() const
{
    return isCalibrating;
}

const std::vector<int>& SliderCalibrationManager::getTuningIndices()
{
    return tuningIndices;
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
                                  noteLength);
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
//
void SliderCalibrationManager::changeTuningIndices() {
//    tuningIndices = generateTuningPattern (0, 39, octaveOrder.at (octaveIdx));
//    for (auto idx : tuningIndices) std::cout << "idx: " << idx << std::endl;
//    octaveIdx++;
//    if (octaveIdx >= octaveOrder.size()) octaveIdx = 0;
//    
//    playTuningNotes();
    
    std::vector<float> frequencies = sliderSetPointManager.getFrequencies();
    int numFrequencies = frequencies.size();

    // Find the minimum play count
    int minCount = std::numeric_limits<int>::max();
    for (const auto& [index, count] : tuningIndexCounts) 
    {
        if (count < minCount) minCount = count;
    }

    // Collect indices with the minimum play count
    std::vector<int> minCountIndices;
    for (const auto& [index, count] : tuningIndexCounts) {
        if (count == minCount) {
            minCountIndices.push_back(index);
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
    
    for (const auto& idx : tuningIndices) 
    {
        subsetFrequencies.push_back(allFrequencies[idx]);
        subsetAmplitudes.push_back(allAmplitudes[idx]);
        subsetPans.push_back(allPans[idx]);
    }
    
    sliderSequencer.playTuningNotes (subsetFrequencies, subsetAmplitudes, subsetPans);
//    sliderSequencer.playRandomNotes (subsetFrequencies, subsetAmplitudes, subsetPans);
}

std::vector<int> SliderCalibrationManager::generateTuningPattern (int start, int end, int octave)
{
    std::vector<int> pattern;
    int numOctaves = end - start + 1;
    int octaveSize = 4;

    int base = octave * octaveSize;
    
    pattern.push_back(base + 0);
    pattern.push_back(base + 2);
    pattern.push_back(base + 3);
    pattern.push_back(base + 1);

    return pattern;
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

int SliderCalibrationManager::goToNextQuestion (float newVal)
{
    std::cout << "Currently playing: " << currNoteIdx << std::endl;
    std::cout << "Locked In Indices before: ";
    for (int i = 0; i < lockedInIndices.size(); ++i) std::cout << lockedInIndices[i] << " ";
    std::cout << std::endl;
    
    std::cout << "Pending Indices before: ";
    for (int i = 0; i < pendingIndices.size(); ++i) std::cout << pendingIndices[i] << " ";
    std::cout << std::endl;
    
    if (nextNoteIdx >= comparisonList.size()) return 0;
    
    // If the adjustment was small, lock in the current note, add a new note to the mix
    float lastVal = sliderSetPointManager.getAmplitudeAt (currNoteIdx);
    float adjustment = abs (lastVal - newVal);
    sliderSetPointManager.setAmplitudeAt (currNoteIdx, newVal);
    
    std::cout << "LastVal: " << lastVal << ", CurrVal: " << newVal << std::endl;
    std::cout << "Adjustment: " << adjustment << std::endl;
    
    std::cout << "amplitude after: " << sliderSetPointManager.getAmplitudeAt (currNoteIdx) << std::endl;
    
    if (adjustment < 0.1)
    {
        std::cout << "LOCKING IN!!" << std::endl;
        // remove from pending indices and add to locked in indices
        for (int i = 0; i < pendingIndices.size(); ++i)
        {
            std::cout << "searching... " << currNoteIdx << std::endl;
            if (pendingIndices[i] == currNoteIdx)
            {
                pendingIndices.erase (pendingIndices.begin() + i);
                lockedInIndices.push_back (currNoteIdx);
                
                std::cout << "LOCKED IN " << currNoteIdx << std::endl;
                
                pendingIndices.push_back (comparisonList[nextNoteIdx]);
                nextNoteIdx++;
                
                break;
            }
        }
    }
    
    
    
    // Generate a new question from pending indices and locked in indices
    std::vector<int> availableIndices;
    for (int i = 0; i < lockedInIndices.size(); ++i) availableIndices.push_back (lockedInIndices[i]);
    
    if (lockedInIndices.size() < 3)
    {
        for (int i = 0; i < pendingIndices.size(); ++i) availableIndices.push_back (pendingIndices[i]);
    }
    
    // Pick random pending index as controlled tone and remove it from available indices
    int currNoteIdx = getRandomElement (pendingIndices);
    int secondIdx = getRandomElement (availableIndices);
    
    while (secondIdx == lastAskedIdx || secondIdx == currNoteIdx)
    {
        currNoteIdx = getRandomElement (pendingIndices);
        secondIdx = getRandomElement (availableIndices);
    }
    removeElementMatching (currNoteIdx, availableIndices);
     
    lastAskedIdx = secondIdx;
    // Now the question is between tones at (currIdx, secondIdx)
    sliderSequencer.playTwoToneInterval (sliderSetPointManager.getFrequencyAt (currNoteIdx),
                                         sliderSetPointManager.getAmplitudeAt (currNoteIdx),
                                         sliderSetPointManager.getPanAt (currNoteIdx),
                                         sliderSetPointManager.getFrequencyAt (secondIdx),
                                         sliderSetPointManager.getAmplitudeAt (secondIdx),
                                         sliderSetPointManager.getPanAt (secondIdx));
    
    std::cout << "Locked In Indices after: ";
    for (int i = 0; i < lockedInIndices.size(); ++i) std::cout << lockedInIndices[i] << " ";
    std::cout << std::endl;
    
    std::cout << "Pending Indices after: ";
    for (int i = 0; i < pendingIndices.size(); ++i) std::cout << pendingIndices[i] << " ";
    std::cout << std::endl;
    
    std::cout << "currNoteIdx after: " << currNoteIdx << std::endl;
    std::cout << std::endl;
    
    return currNoteIdx;
}

void SliderCalibrationManager::updateReferenceTone()
{
    sliderSequencer.setReferenceNote (sliderSetPointManager.getFrequencyAt (referenceToneIdx),
                                      sliderSetPointManager.getAmplitudeAt (referenceToneIdx));
}

int SliderCalibrationManager::getRandomElement (std::vector<int> vec) const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, static_cast<int> (vec.size()) - 1);
    int random_idx = distr(gen);
    
    return vec[random_idx];
}

int SliderCalibrationManager::removeElementMatching (int val, std::vector<int>& vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        if (vec[i] == val)
        {
            vec.erase (vec.begin() + i);
            break;
        }
    }
}
