/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilterPage.h"
#include "CabinEQPage.h"
#include "ReferenceCalibrationPage.h"

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
    std::unique_ptr<CabinEQPage> cabinEQPage;
    std::unique_ptr<ReferenceCalibrationPage> referenceCalibrationPage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartupMVPAudioProcessorEditor)
};
