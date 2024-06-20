/*
  ==============================================================================

    SineWaveGenerator.h
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SineWaveGenerator : juce::Timer
{
public:
    SineWaveGenerator ();
    
    void setSampleRate (double newSampleRate);
    void setFrequency (double frequency);
    double getNextSample ();
    double getFrequency () { return frequency; }
    double getIsPlayingReferenceFrequency () { return isPlayingReferenceFrequency; }
    
    void timerCallback() override;
    
    
private:
    void startNote();
    void updatePhaseAndAmplitude();
    
    double tilt = 0.6; // 0.5 ~ pink noise, 0.6 ~ equal loudness, 0.4 ~ bassier
    
    double sampleRate = 44100;
    double currentFrequency = 1000;
    double nextFrequency = -1;
    double frequency = 1000;
    double referenceFrequency = 1000;
    double phase = 0;
    double phaseIncrement = 0;
    double amplitudeScale = 0;
    
    bool isPlayingReferenceFrequency = false;
    
    int gainRamp = 0;
    
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
};
