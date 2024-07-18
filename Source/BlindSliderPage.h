/*
  ==============================================================================

    BlindSliderPage.h
    Created: 17 Jul 2024 11:13:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BlindSliderPage   : juce::Component,
                          juce::Slider::Listener,
                          juce::Button::Listener
{
public:
    BlindSliderPage (StartupMVPAudioProcessor& p) : processor (p) {}
    
private:
    void parameterChangedCallback (float newValue, int idx);
    std::array<std::unique_ptr<juce::ParameterAttachment>, SliderSetPointManager::NUM_PTS> parameterAttachments;
    
    StartupMVPAudioProcessor& processor;
    
    juce::Slider slider;
    juce::TextButton calibrationToggleButton { "Start Calibration" };
    juce::TextButton nextButton { "Next >" };
    juce::Label progressLabel;
    
    int currIdx = 0;
    bool isCalibrating = false;
};
