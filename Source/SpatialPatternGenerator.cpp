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

void SpatialPatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    noiseGenerator.setSampleRate (spec.sampleRate);
    setPeakFilter (1000, 1, 0);
    leftPeakFilter.prepare (spec);
    rightPeakFilter.prepare (spec);
}

void SpatialPatternGenerator::setPattern (std::vector<NoiseNote> notes)
{
    this->notes = notes;
    currNoteIdx = 0;
    for (const auto& note : notes)
    {
        std::cout << "ampl: " << note.ampl << std::endl;
    }
    numSamplesNoteHasBeenPlaying = 0;
    updateBandpassAndPanning();
}

void SpatialPatternGenerator::setPeakFilter (float centerFreq, float bandwidth, float ampl)
{
    float qFactor = Band::bandwidthToQFactor (bandwidth);
    *leftPeakFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, centerFreq, qFactor, juce::Decibels::decibelsToGain (ampl));
    *rightPeakFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, centerFreq, qFactor, juce::Decibels::decibelsToGain (ampl));
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
//    return noiseGenerator.getNextSample();
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return { 0, 0 };
    
    std::pair<float, float> sample = noiseGenerator.getNextSample();
    sample.first = leftPeakFilter.processSample (sample.first);
    sample.second = rightPeakFilter.processSample (sample.second);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples)
    {
        goToNextNote();
    }
    
    auto [leftEnvelopeGain, rightEnvelopeGain] = getCurrNote().getGainAtSample (numSamplesNoteHasBeenPlaying);
    
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
    float angle = (getCurrNote().pan + 1.0f) * M_PI / 4.0f; // Map pan from [-1, 1] to angle [0, π/2]
    leftGain = std::cos(angle);
    rightGain = std::sin(angle);
    float gainChange = std::min (juce::Decibels::decibelsToGain (getCurrNote().ampl), 1.0f);
    leftGain *= gainChange;
    rightGain *= gainChange;
    
    std::cout << "getCurrNote().ampl: " << getCurrNote().ampl << std::endl;
    
//    leftGain = 1.0f;
//    rightGain = 1.0f;
    
    float bandwidth = getCurrNote().bandwidth;
    if (bandwidth != 0)
    {
        noiseGenerator.setBandpass (bandpassFrequency, getCurrNote().bandwidth);
    }
    else
    {
        leftGain = 0;
        rightGain = 0;
    }
}
