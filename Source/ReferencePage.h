/*
  ==============================================================================

    ReferencePage.h
    Created: 24 Aug 2024 10:26:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ReferencePage   : public juce::Component,
                        public juce::Slider::Listener
{
public:
    ReferencePage(StartupMVPAudioProcessor& p);
    ~ReferencePage() override;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    juce::Slider referenceVolume1;
    juce::Slider referenceVolume2;
};
