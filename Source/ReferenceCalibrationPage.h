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
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
private:
    StartupMVPAudioProcessor& processor;
    
    juce::Slider slider_referenceFreq1;
    juce::Slider slider_referenceFreq2;
    juce::Slider slider_referenceAmplLeft1;
    juce::Slider slider_referenceAmplLeft2;
    juce::Slider slider_referenceAmplRight1;
    juce::Slider slider_referenceAmplRight2;
    juce::Slider slider_referenceCrossfeedGainLeft1;
    juce::Slider slider_referenceCrossfeedGainRight1;
    juce::Slider slider_referenceCrossfeedGainLeft2;
    juce::Slider slider_referenceCrossfeedGainRight2;
    
    juce::Label label_referenceFreq1;
    juce::Label label_referenceFreq2;
    juce::Label label_referenceAmplLeft1;
    juce::Label label_referenceAmplLeft2;
    juce::Label label_referenceAmplRight1;
    juce::Label label_referenceAmplRight2;
    juce::Label label_referenceCrossfeedGainLeft1;
    juce::Label label_referenceCrossfeedGainRight1;
    juce::Label label_referenceCrossfeedGainLeft2;
    juce::Label label_referenceCrossfeedGainRight2;
    
    juce::Slider slider_delayInMs;
    juce::Label label_delayInMs;
    
    Channel currChannel = Channel::LEFT;
};
