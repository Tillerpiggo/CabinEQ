/*
  ==============================================================================

    SineWaveChordGenerator.cpp
    Created: 6 Sep 2024 1:37:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveChordGenerator.h"
#include <algorithm>
#include <random>

SineWaveChordGenerator::SineWaveChordGenerator()
{}

std::pair<float, float> SineWaveChordGenerator::getNextSample()
{
    auto leftSample = 0;
    auto rightSample = 0;
    for (auto& sequencer : sequencers)
    {
        auto [nextLeftSample, nextRightSample] = sequencer.getNextSample();
        leftSample += nextLeftSample;
        rightSample += nextRightSample;
    }
    
    return { leftSample, rightSample };
}

void SineWaveChordGenerator::setSampleRate (float newSampleRate)
{
    this->sampleRate = newSampleRate;
    prepareSequencers(); // to update them with the new sample rate
}

void SineWaveChordGenerator::setPlayingChord (Curve& curve, float freqToExclude, int numTones)
{
    StereoGainEnvelope envelope (500);
    std::cout << "set playing chord!!!" << std::endl;
    
    int numTonesInCurve = static_cast<int> (curve.getCurvePts().size());
    int numTonesToPlay;
    if (numTones == -1)
        numTonesToPlay = numTonesInCurve;
    else
        numTonesToPlay = std::min (numTones, numTonesInCurve);
    
    std::cout << "(1) num tones to play = " << numTonesToPlay << std::endl;
    
    // Make sure we have enough sequencers to play the tones
    if (sequencers.size() < numTonesToPlay)
    {
        std::cout << "(1.1) trying to add sequencers" << std::endl;
        size_t numSequencersToAdd = numTonesToPlay - sequencers.size();
        for (size_t i = 0; i < numSequencersToAdd; ++i)
        {
            // Add a new sequencer with a SineWaveGenerator
            sequencers.emplace_back (std::make_unique<SineWaveGenerator> (SineWaveGenerator()));
        }
    }
    std::cout << "(1.4) added sequencers" << std::endl;
    prepareSequencers(); // let's make sure they all have the latest sample rate
    
    std::cout << "(1.5) prepared sequencers" << std::endl;
    
    // Get the notes for each sequencer
    auto notesToPlay = getNotesToPlay (curve, numTonesToPlay);
    
    std::cout << "(2) num tones to play = " << numTonesToPlay << std::endl;
    
    // Load up the sequencers
    for (int i = 0; i < numTonesToPlay; ++i)
    {
        std::vector<SequenceableNote> notesToPlayForSequencer;
        for (const auto& node : notesToPlay[i])
        {
            auto ampl = node.second == freqToExclude ? 0.0f : node.second;
            notesToPlayForSequencer.emplace_back (node.first, ampl, 0.0f, M_PI, 2000, envelope);
        }
        sequencers[i].setNotes (notesToPlayForSequencer);
        std::cout << "setting " << notesToPlayForSequencer.size() << " notes" << std::endl;
    }
}

void SineWaveChordGenerator::updatePlayingChord (Curve& curve, float freqToExclude, int numTones)
{
    StereoGainEnvelope envelope (500);
    
    int numTonesInCurve = static_cast<int> (curve.getCurvePts().size());
    int numTonesToPlay;
    if (numTones == -1)
        numTonesToPlay = numTonesInCurve;
    else
        numTonesToPlay = std::min (numTones, numTonesInCurve);
    
    // we should already have enough sequencers to play the tones...
    
    // Do the same as in startPlaying chord, but update notes instead
    auto notesToPlay = getNotesToPlay (curve, numTonesToPlay);
    
    for (int i = 0; i < numTonesToPlay; ++i)
    {
        std::vector<SequenceableNote> notesToPlayForSequencer;
        for (const auto& node : notesToPlay[i])
        {
            auto ampl = node.second == freqToExclude ? 0.0f : node.second;
            notesToPlayForSequencer.emplace_back (node.first, ampl, 0.0f, M_PI, 2000, envelope);
        }
        sequencers[i].updateNotes (notesToPlayForSequencer);
    }
}

std::vector<std::vector<std::pair<float, float>>> SineWaveChordGenerator::getNotesToPlay (Curve& curve, int numTonesToPlay)
{
    // Get the notes for each sequencer
    std::vector<std::vector<std::pair<float, float>>> notesToPlay;
    for (int i = 0; i < numTonesToPlay; ++i)
        notesToPlay.push_back (std::vector<std::pair<float, float>>());
    
    for (int i = 0; i < 8; ++i) // generate 8 random batches of tones
    {
        auto allNodesOrig = curve.getCurvePts();
        std::vector<CurvePt> allNodes = allNodesOrig; // make a copy of it
        
        // Shuffle it
        std::random_device rd;
        std::mt19937 eng(rd());
        std::shuffle (allNodes.begin(), allNodes.end(), eng);
        
        // Get numTonesToPlay tone pairs
        std::vector<std::pair<float, float>> batch; // a node for each sequencer
        for (int i = 0; i < numTonesToPlay; ++i)
        {
            notesToPlay[i].emplace_back (allNodes[i].freq, allNodes[i].val);
        }
    }
    
    for (const auto& note : notesToPlay[0])
        std::cout << "notefreq: " << note.first << std::endl;
    
    return notesToPlay;
}

void SineWaveChordGenerator::prepareSequencers()
{
    for (auto& sequencer : sequencers)
    {
        std::cout << "(1.2) adding sequencer" << std::endl;
        sequencer.setSampleRate (sampleRate);
        sequencer.setNotes ({}); // stop playing whatever we were playing
        std::cout << "(1.2.1) added sequencer" << std::endl;
    }
    
    std::cout << "(1.3) fininshed preparing sequencers" << std::endl;
}
