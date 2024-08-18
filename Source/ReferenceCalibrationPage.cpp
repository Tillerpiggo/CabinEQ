/*
  ==============================================================================

    ReferenceCalibrationPage.cpp
    Created: 18 Aug 2024 1:15:26am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ReferenceCalibrationPage.h"

ReferenceCalibrationPage::ReferenceCalibrationPage (StartupMVPAudioProcessor& p)
    : processor (p)
{
    juce::Slider slider_referenceFreq1;
    juce::Slider slider_referenceFreq2;
    juce::Slider slider_referenceAmplLeft1;
    juce::Slider slider_referenceAmplLeft2;
    juce::Slider slider_referenceAmplRight1;
    juce::Slider slider_referenceAmplRight2;
    
    slider_referenceFreq1.addListener (this);
    slider_referenceFreq2.addListener (this);
    slider_referenceAmplLeft1.addListener (this);
    slider_referenceAmplLeft2.addListener (this);
    slider_referenceAmplRight1.addListener (this);
    slider_referenceAmplRight2.addListener (this);
    
    addAndMakeVisible (slider_referenceFreq1);
    addAndMakeVisible (slider_referenceFreq2);
    addAndMakeVisible (slider_referenceAmplLeft1);
    addAndMakeVisible (slider_referenceAmplLeft2);
    addAndMakeVisible (slider_referenceAmplRight1);
    addAndMakeVisible (slider_referenceAmplRight2);
}

ReferenceCalibrationPage::~ReferenceCalibrationPage()
{
    slider_referenceFreq1.removeListener (this);
    slider_referenceFreq2.removeListener (this);
    slider_referenceAmplLeft1.removeListener (this);
    slider_referenceAmplLeft2.removeListener (this);
    slider_referenceAmplRight1.removeListener (this);
    slider_referenceAmplRight2.removeListener (this);
}

void ReferenceCalibrationPage::paint (juce::Graphics&)
{
    // TODO
}

void ReferenceCalibrationPage::resized()
{
    int sliderHeight = getHeight() / 6;
    int padding = 20;
    int sliderWidth = getWidth() - padding * 2;
    
    slider_referenceFreq1.setBounds (padding, sliderHeight * 0, sliderWidth, sliderHeight);
    slider_referenceFreq2.setBounds (padding, sliderHeight * 1, sliderWidth, sliderHeight);
    slider_referenceAmplLeft1.setBounds (padding, sliderHeight * 2, sliderWidth, sliderHeight);
    slider_referenceAmplLeft2.setBounds (padding, sliderHeight * 3, sliderWidth, sliderHeight);
    slider_referenceAmplRight1.setBounds (padding, sliderHeight * 4, sliderWidth, sliderHeight);
    slider_referenceAmplRight2.setBounds (padding, sliderHeight * 5, sliderWidth, sliderHeight);
}

void ReferenceCalibrationPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &slider_referenceFreq1)
    {
        processor.setReferenceFreq1 (slider_referenceFreq1.getValue());
    }
    else if (slider == &slider_referenceFreq2)
    {
        processor.setReferenceFreq2 (slider_referenceFreq2.getValue());
    }
    else if (slider == &slider_referenceAmplLeft1)
    {
        processor.setReferenceAmplLeft1 (slider_referenceAmplLeft1.getValue());
    }
    else if (slider == &slider_referenceAmplLeft2)
    {
        processor.setReferenceAmplLeft2 (slider_referenceAmplLeft2.getValue());
    }
    else if (slider == &slider_referenceAmplRight1)
    {
        processor.setReferenceAmplRight1 (slider_referenceAmplRight1.getValue());
    }
    else if (slider == &slider_referenceAmplRight2)
    {
        processor.setReferenceAmplRight2 (slider_referenceAmplRight2.getValue());
    }
}
