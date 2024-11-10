///*
//  ==============================================================================
//
//    MultiplePatternGenerator.cpp
//    Created: 6 Nov 2024 1:08:19am
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#include "MultiplePatternGenerator.h"
//
//MultiplePatternGenerator::MultiplePatternGenerator()
//{
//}
//
//
//std::pair<float, float> MultiplePatternGenerator::getNextSample()
//{
//    std::pair<float, float> nextSample { 0.0f, 0.0f };
//    
//    if (isMuted)
//        return nextSample;
//    
//    for (auto& noiseGenerator : noiseGenerators)
//    {
////        std::cout << "noise generator! " << std::endl;
//        auto noiseGeneratorSample = noiseGenerator->getNextSample();
//        nextSample.first += noiseGeneratorSample.first * 5.0f;
//        nextSample.second += noiseGeneratorSample.second * 5.0f;
//    }
//    
//    return nextSample;
//}
//
//void MultiplePatternGenerator::prepare (const juce::dsp::ProcessSpec& spec)
//{
//    this->spec = spec;
//}
//
//void MultiplePatternGenerator::setSpeedFactor (float speedFactor)
//{
//    // TODO: Implement
//}
//
//void MultiplePatternGenerator::setFreqFactor (float freqFactor)
//{
//    this->freqFactor = freqFactor;
//    setPattern (pattern.value());
//}
//
//void MultiplePatternGenerator::mute()
//{
//    isMuted = true;
//}
//
//
//void MultiplePatternGenerator::setPattern (FauxMusicPattern fauxMusicPattern)
//{
//    // For all existing generators, simply set their pattern
//    for (int i = 0; i < noiseGenerators.size(); ++i)
//    {
//        noiseGenerators[i]->setPattern (fauxMusicPattern.getPatterns()[i]);
//        auto shiftedFreqRange = fauxMusicPattern.getFreqRanges()[i];
//        shiftedFreqRange.first *= freqFactor;
//        shiftedFreqRange.second *= freqFactor;
//        noiseGenerators[i]->setFrequencyRange (shiftedFreqRange);
//    }
//    
//    // For any new needed generators, add them, prepare them, and then set their pattern
//    int numToAdd = static_cast<int> (fauxMusicPattern.getNumPatterns()) - static_cast<int> (noiseGenerators.size());
//    std::cout << "num to add: " << numToAdd << std::endl;
//    int numNoiseGenerators = static_cast<int> (noiseGenerators.size());
//    for (int i = 0; i < numToAdd; ++i)
//    {
//        std::cout << "adding noise generator" << std::endl;
//        int idx = i + numNoiseGenerators;
//        noiseGenerators.push_back (std::make_unique<NoiseGenerator>());
//        noiseGenerators[idx]->setPattern (fauxMusicPattern.getPatterns()[i]);
//        noiseGenerators[idx]->setFrequencyRange (fauxMusicPattern.getFreqRanges()[i]);
//        noiseGenerators[idx]->prepare (spec);
//    }
//    
//    isMuted = false;
//    this->pattern = fauxMusicPattern;
//}
