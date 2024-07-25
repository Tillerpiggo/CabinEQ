/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilterPage.h"
#include "SliderPage.h"
#include "LeftRightPage.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessorEditor   : public juce::AudioProcessorEditor
{
public:
    StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor&);
    ~StartupMVPAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    StartupMVPAudioProcessor& audioProcessor;

    juce::TabbedComponent tabbedComponent;
    
    std::unique_ptr<FilterPage> filterPage;
    std::unique_ptr<SliderPage> sliderPage;
    std::unique_ptr<LeftRightPage> leftRightPage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartupMVPAudioProcessorEditor)
};
