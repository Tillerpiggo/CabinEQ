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
#include "ReferencePage.h"
//#include "SpatialCalibrationPage.h"

//==============================================================================
/**
*/
class CabinEQProcessorEditor   : public juce::AudioProcessorEditor
{
public:
    CabinEQProcessorEditor (CabinEQAudioProcessor&);
    ~CabinEQProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CabinEQAudioProcessor& audioProcessor;
    
    CabinEQPage cabinEQPage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinEQProcessorEditor)
};
