/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CurveComponent.h"
#include "SliderGroup.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public juce::Button::Listener
{
public:
    StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor&);
    ~StartupMVPAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void buttonClicked (juce::Button *button) override;
    
private:
    void addSliderPair (int i);
    juce::Slider& addSlider (std::string name, std::string paramName, int idx);
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StartupMVPAudioProcessor& audioProcessor;
    
    juce::TabbedComponent tabbedComponent;
    std::vector<std::unique_ptr<SliderGroup>> sliderGroups;
    
    juce::TextButton prevButton { "< PREV" };
    juce::TextButton nextButton { "NEXT >" };
    juce::TextButton bypassButton { "BYPASS" };
    
    CurveComponent curveComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessorEditor)
};
