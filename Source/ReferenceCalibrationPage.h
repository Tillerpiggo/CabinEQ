/*
  ==============================================================================

    ReferenceCalibrationPage.h
    Created: 18 Aug 2024 1:15:26am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// This class contains sliders to set the reference values for a given profile
class ReferenceCalibrationPage   : public juce::Component,
                                   public juce::Slider::Listener
{
public:
    ReferenceCalibrationPage (StartupMVPAudioProcessor& p);
    ~ReferenceCalibrationPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    juce::Slider slider_referenceFreq1;
    juce::Slider slider_referenceFreq2;
    juce::Slider slider_referenceAmplLeft1;
    juce::Slider slider_referenceAmplLeft2;
    juce::Slider slider_referenceAmplRight1;
    juce::Slider slider_referenceAmplRight2;
};
