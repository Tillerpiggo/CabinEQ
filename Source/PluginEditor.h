/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CurveComponent.h"


//==============================================================================
/**
*/
class StartupMVPAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor&);
    ~StartupMVPAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void addSlider(std::string name, std::string paramName, int idx);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StartupMVPAudioProcessor& audioProcessor;
    juce::Viewport viewport;
    juce::Component sliderContainer;
    
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    
    CurveComponent curveComponent;
    CurveComponent balanceCurveComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessorEditor)
};
