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
    // Set ranges for each slider
    slider_referenceFreq1.setRange (20.0f, 20000.0f);
    slider_referenceFreq2.setRange (20.0f, 20000.0f);
    slider_referenceAmplLeft1.setRange (-12.0f, 12.0f);
    slider_referenceAmplLeft2.setRange (-12.0f, 12.0f);
    slider_referenceAmplRight1.setRange (-12.0f, 12.0f);
    slider_referenceAmplRight2.setRange (-12.0f, 12.0f);
    
    // Make each slider horizontal
    slider_referenceFreq1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceFreq2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight2.setSliderStyle (juce::Slider::LinearHorizontal);
    
    // TODO: Add labels to each slider (figure out how to do this)
    slider_referenceFreq1.setHelpText("help");
    slider_referenceFreq2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight2.setSliderStyle (juce::Slider::LinearHorizontal);
    
    // Add ourselves as a listener to each slider
    slider_referenceFreq1.addListener (this);
    slider_referenceFreq2.addListener (this);
    slider_referenceAmplLeft1.addListener (this);
    slider_referenceAmplLeft2.addListener (this);
    slider_referenceAmplRight1.addListener (this);
    slider_referenceAmplRight2.addListener (this);
    
    // Make each slider visible
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
    
    processor.updateCalibratingEQNode (EQNode (0, 0, 0), Channel::LEFT);
}

void ReferenceCalibrationPage::sliderDragStarted (juce::Slider *slider)
{
    processor.startCalibratingEQNode (EQNode (0, 0, 0), Channel::LEFT);
}

void ReferenceCalibrationPage::sliderDragEnded (juce::Slider *slider)
{
    processor.endCalibratingEQNode();
}
