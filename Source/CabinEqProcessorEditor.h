/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "CabinEqPage.h"

//==============================================================================
/**
*/
class CabinEqProcessorEditor   : public juce::AudioProcessorEditor
{
public:
    CabinEqProcessorEditor (CabinEqAudioProcessor&);
    ~CabinEqProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CabinEqAudioProcessor& audioProcessor;
    
    CabinEqPage cabinEqPage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinEqProcessorEditor)
};
