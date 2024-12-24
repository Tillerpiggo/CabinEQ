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
class CabinEqProcessorEditor   : public juce::AudioProcessorEditor,
                                 public juce::DragAndDropContainer,
                                 public juce::Button::Listener
{
public:
    CabinEqProcessorEditor (CabinEqAudioProcessor&);
    ~CabinEqProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void buttonClicked (juce::Button* button) override;

private:
    CabinEqAudioProcessor& audioProcessor;
    
    CabinEqPage cabinEqPage;
    juce::TextButton visibilityButton { "VISIBLE" };
    bool isVisible = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinEqProcessorEditor)
};
