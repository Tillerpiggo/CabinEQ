/*
  ==============================================================================

    StupidExperimentPage.h
    Created: 30 Jul 2024 6:21:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StupidExperimentPage   : public juce::Component,
                               public juce::Slider::Listener,
                               public juce::Button::Listener,
                               public juce::Timer
{
public:
    StupidExperimentPage (StartupMVPAudioProcessor& p);
    ~StupidExperimentPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    void timerCallback() override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    static constexpr float MIN_FREQ = 100;
    static constexpr float MAX_FREQ = 19000;
    
    float currFreq = MIN_FREQ;
    float step = 0;
    
    juce::TextButton startButton { "Start" };
    juce::Slider slider;
};
