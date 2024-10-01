/*
  ==============================================================================

    SpatialPatternGenerator.cpp
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialPatternGenerator.h"

SpatialPatternGenerator::SpatialPatternGenerator()
    : numSamplesNoteHasBeenPlaying (0)
{
}

void SpatialPatternGenerator::setSampleRate (float sampleRate)
{
    noiseGenerator.setSampleRate (sampleRate);
    this->sampleRate = sampleRate;
}

void SpatialPatternGenerator::setAmplCurve (Curve& amplCurve)
{
    noiseGenerator.setAmplCurve (amplCurve);
}

void SpatialPatternGenerator::setPattern (std::vector<NoiseNote> notes)
{
    this->notes = notes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    updateBandpassAndPanning();
}

void SpatialPatternGenerator::setMelodicPattern (std::vector<int> notesInSemitones, std::vector<float> pans, float centerFreq, float bandwidth, float noteDurationInMs)
{
    std::vector<NoiseNote> noiseNotes;
    
    float semitoneRatio = std::pow (2.0f, 1.0f / 12.0f);
    int noteDurationInSamples = (noteDurationInMs / 1000.0f) * sampleRate;
    for (int i = 0; i < notesInSemitones.size(); ++i)
    {
        float noteFreq = centerFreq * std::pow (semitoneRatio, notesInSemitones[i]);
        noiseNotes.push_back (NoiseNote(noteFreq, bandwidth, noteDurationInSamples, pans[i], { 1.0f, 1.0f }, false));
    }
    
    setPattern (noiseNotes);
}

void SpatialPatternGenerator::setMelodicPattern (std::vector<int> notesInSemitones, float centerFreq, float bandwidth, float noteDurationInMs)
{
    std::vector<float> pans (notesInSemitones.size(), 0.0f);
    setMelodicPattern (notesInSemitones, pans, centerFreq, bandwidth, noteDurationInMs);
}

void SpatialPatternGenerator::setCenterFrequency (float centerFrequency)
{
    this->centerFrequency = centerFrequency;
}

std::pair<float, float> SpatialPatternGenerator::getNextSample()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return { 0, 0 };
    
    std::pair<float, float> sample = noiseGenerator.getNextSample();
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples)
    {
        goToNextNote();
    }
    
    auto [leftEnvelopeGain, rightEnvelopeGain] = getCurrNote().getGainAtSample (numSamplesNoteHasBeenPlaying);
    
    leftEnvelopeGain = 1;
    rightEnvelopeGain = 1;
    // Apply panning
    return { sample.first * leftGain * leftEnvelopeGain, sample.second * rightGain * rightEnvelopeGain };
}

NoiseNote SpatialPatternGenerator::getCurrNote()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return notes[0];
    return notes[currNoteIdx];
}

void SpatialPatternGenerator::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
    {
        currNoteIdx = 0;
    }
    
    updateBandpassAndPanning();
}

void SpatialPatternGenerator::updateBandpassAndPanning()
{
    float bandpassFrequency = getCurrNote().freqFactor;
    if (getCurrNote().isRelativeToCenterFrequency)
        bandpassFrequency *= centerFrequency;
    
    // Adjust leftgain and rightgain according to the curr note's angle
//    float angle = getCurrNote().pan * M_PI / 4.0f; // go from [-1, 1] to [-pi/4, pi/4]
//    leftGain = std::sqrt (2.0f) / 2.0f * (std::cos (angle) - std::sin(angle));
//    rightGain = std::sqrt (2.0f) / 2.0f * (std::cos(angle) + std::sin(angle));
//    leftGain *= juce::Decibels::decibelsToGain (getCurrNote().ampl);
//    rightGain *= juce::Decibels::decibelsToGain (getCurrNote().ampl);
    
    float angle = (getCurrNote().pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
    leftGain = std::cos(angle);
    rightGain = std::sin(angle);
    float gainChange = std::min (juce::Decibels::decibelsToGain (getCurrNote().ampl), 1.0f);
    leftGain *= gainChange;
    rightGain *= gainChange;
    
    noiseGenerator.setBandpass (bandpassFrequency, getCurrNote().bandwidth, getCurrNote().bandwidthEnvelope.first, getCurrNote().bandwidthEnvelope.second);
}
