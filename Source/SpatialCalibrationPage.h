/*
  ==============================================================================

    SpatialCalibrationPage.h
    Created: 18 Aug 2024 9:49:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SpatialCalibrationPage   : public juce::Component,
                                 public juce::Slider::Listener,
                                 public juce::Button::Listener,
                                 public StartupMVPAudioProcessor::Listener
{
public:
    SpatialCalibrationPage (StartupMVPAudioProcessor& p);
    ~SpatialCalibrationPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    void sliderValueChanged (juce::Slider *slider) override;
    void buttonClicked (juce::Button *button) override;
    
    void didLoadData() override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton prevButton { "PREV" };
    juce::TextButton nextButton { "NEXT" };
    
    juce::String profileId { "SPATIAL" };
    
    std::vector<float> freqs;
    int currNodeId = 0;
};
