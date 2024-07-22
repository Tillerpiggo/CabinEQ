/*
  ==============================================================================

    VerifyFilterPage.h
    Created: 20 Jul 2024 3:25:29pm
    Author:  Tyler Gee

  ==============================================================================
*/


#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VerifyFilterPage   : public juce::Component,
                           public juce::Slider::Listener,
                           public juce::Button::Listener
{
public:
    VerifyFilterPage (StartupMVPAudioProcessor& p);
    ~VerifyFilterPage() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
    void buttonClicked (juce::Button *button) override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    juce::Slider tone1FreqSlider;
    juce::Slider tone1VolSlider;
    juce::Slider tone2FreqSlider;
    juce::Slider tone2VolSlider;
    juce::Slider filterVolumeSlider;
    
    juce::TextButton togglePlayingButton { "Start Playing" };
    juce::TextButton toggleFilterButton { "Enable Filter" };
    
    bool isTesting = false;
    bool isFilterEnabled = false;
};

