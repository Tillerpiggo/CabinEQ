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
    slider_referenceCrossfeedGainLeft1.setRange (0.0f, 1.0f);
    slider_referenceCrossfeedGainRight1.setRange (0.0f, 1.0f);
    slider_referenceCrossfeedGainLeft2.setRange (0.0f, 1.0f);
    slider_referenceCrossfeedGainRight2.setRange (0.0f, 1.0f);
    slider_delayInMs.setRange (0, 0.7);

    // Make each slider horizontal
    slider_referenceFreq1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceFreq2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplLeft2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceAmplRight2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceCrossfeedGainLeft1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceCrossfeedGainRight1.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceCrossfeedGainLeft2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_referenceCrossfeedGainRight2.setSliderStyle (juce::Slider::LinearHorizontal);
    slider_delayInMs.setSliderStyle (juce::Slider::LinearHorizontal);

    // Add labels to each slider
    label_referenceFreq1.setText("Reference Frequency 1", juce::dontSendNotification);
    label_referenceFreq1.attachToComponent(&slider_referenceFreq1, false);

    label_referenceFreq2.setText("Reference Frequency 2", juce::dontSendNotification);
    label_referenceFreq2.attachToComponent(&slider_referenceFreq2, false);

    label_referenceAmplLeft1.setText("Reference Amplitude Left 1", juce::dontSendNotification);
    label_referenceAmplLeft1.attachToComponent(&slider_referenceAmplLeft1, false);

    label_referenceAmplLeft2.setText("Reference Amplitude Left 2", juce::dontSendNotification);
    label_referenceAmplLeft2.attachToComponent(&slider_referenceAmplLeft2, false);

    label_referenceAmplRight1.setText("Reference Amplitude Right 1", juce::dontSendNotification);
    label_referenceAmplRight1.attachToComponent(&slider_referenceAmplRight1, false);

    label_referenceAmplRight2.setText("Reference Amplitude Right 2", juce::dontSendNotification);
    label_referenceAmplRight2.attachToComponent(&slider_referenceAmplRight2, false);

    label_referenceCrossfeedGainLeft1.setText("Crossfeed Gain Left 1", juce::dontSendNotification);
    label_referenceCrossfeedGainLeft1.attachToComponent(&slider_referenceCrossfeedGainLeft1, false);

    label_referenceCrossfeedGainRight1.setText("Crossfeed Gain Right 1", juce::dontSendNotification);
    label_referenceCrossfeedGainRight1.attachToComponent(&slider_referenceCrossfeedGainRight1, false);

    label_referenceCrossfeedGainLeft2.setText("Crossfeed Gain Left 2", juce::dontSendNotification);
    label_referenceCrossfeedGainLeft2.attachToComponent(&slider_referenceCrossfeedGainLeft2, false);

    label_referenceCrossfeedGainRight2.setText("Crossfeed Gain Right 2", juce::dontSendNotification);
    label_referenceCrossfeedGainRight2.attachToComponent(&slider_referenceCrossfeedGainRight2, false);
    
    label_delayInMs.setText ("Crossfeed Delay (ms)", juce::dontSendNotification);
    label_delayInMs.attachToComponent (&slider_delayInMs, false);

    // Add ourselves as a listener to each slider
    slider_referenceFreq1.addListener (this);
    slider_referenceFreq2.addListener (this);
    slider_referenceAmplLeft1.addListener (this);
    slider_referenceAmplLeft2.addListener (this);
    slider_referenceAmplRight1.addListener (this);
    slider_referenceAmplRight2.addListener (this);
    slider_referenceCrossfeedGainLeft1.addListener (this);
    slider_referenceCrossfeedGainRight1.addListener (this);
    slider_referenceCrossfeedGainLeft2.addListener (this);
    slider_referenceCrossfeedGainRight2.addListener (this);
    slider_delayInMs.addListener (this);

    // Make each slider visible
    addAndMakeVisible (slider_referenceFreq1);
    addAndMakeVisible (slider_referenceFreq2);
    addAndMakeVisible (slider_referenceAmplLeft1);
    addAndMakeVisible (slider_referenceAmplLeft2);
    addAndMakeVisible (slider_referenceAmplRight1);
    addAndMakeVisible (slider_referenceAmplRight2);
    addAndMakeVisible (slider_referenceCrossfeedGainLeft1);
    addAndMakeVisible (slider_referenceCrossfeedGainRight1);
    addAndMakeVisible (slider_referenceCrossfeedGainLeft2);
    addAndMakeVisible (slider_referenceCrossfeedGainRight2);
    addAndMakeVisible (slider_delayInMs);
}

ReferenceCalibrationPage::~ReferenceCalibrationPage()
{
    slider_referenceFreq1.removeListener (this);
    slider_referenceFreq2.removeListener (this);
    slider_referenceAmplLeft1.removeListener (this);
    slider_referenceAmplLeft2.removeListener (this);
    slider_referenceAmplRight1.removeListener (this);
    slider_referenceAmplRight2.removeListener (this);
    slider_delayInMs.removeListener (this);
}

void ReferenceCalibrationPage::paint (juce::Graphics&)
{
    // TODO
}

void ReferenceCalibrationPage::resized()
{
    int sliderHeight = getHeight() / 12; // Increased to accommodate the button
    int padding = 20;
    int midPoint = getWidth() / 2;
    int sliderWidth = (getWidth() / 2) - padding * 2;
    
    // Left side sliders
    slider_referenceFreq1.setBounds (padding, sliderHeight * 0, sliderWidth, sliderHeight);
    slider_referenceFreq2.setBounds (padding, sliderHeight * 1, sliderWidth, sliderHeight);
    slider_referenceAmplLeft1.setBounds (padding, sliderHeight * 2, sliderWidth, sliderHeight);
    slider_referenceAmplLeft2.setBounds (padding, sliderHeight * 3, sliderWidth, sliderHeight);
    slider_referenceCrossfeedGainLeft1.setBounds (padding, sliderHeight * 6, sliderWidth, sliderHeight);
    slider_referenceCrossfeedGainLeft2.setBounds (padding, sliderHeight * 8, sliderWidth, sliderHeight);
    
    // Right side sliders
    slider_referenceAmplRight1.setBounds (midPoint + padding, sliderHeight * 4, sliderWidth, sliderHeight);
    slider_referenceAmplRight2.setBounds (midPoint + padding, sliderHeight * 5, sliderWidth, sliderHeight);
    slider_referenceCrossfeedGainRight1.setBounds (midPoint + padding, sliderHeight * 7, sliderWidth, sliderHeight);
    slider_referenceCrossfeedGainRight2.setBounds (midPoint + padding, sliderHeight * 9, sliderWidth, sliderHeight);
    slider_delayInMs.setBounds (midPoint + padding, sliderHeight * 10, sliderWidth, sliderHeight);
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
        currChannel = Channel::LEFT;
    }
    else if (slider == &slider_referenceAmplLeft2)
    {
        processor.setReferenceAmplLeft2 (slider_referenceAmplLeft2.getValue());
        currChannel = Channel::LEFT;
    }
    else if (slider == &slider_referenceAmplRight1)
    {
        processor.setReferenceAmplRight1 (slider_referenceAmplRight1.getValue());
        currChannel = Channel::RIGHT;
    }
    else if (slider == &slider_referenceAmplRight2)
    {
        processor.setReferenceAmplRight2 (slider_referenceAmplRight2.getValue());
        currChannel = Channel::RIGHT;
    }
    else if (slider == &slider_referenceCrossfeedGainLeft1)
    {
        processor.setReferenceCrossfeedGainLeft1 (slider_referenceCrossfeedGainLeft1.getValue());
        currChannel = Channel::LEFT;
    }
    else if (slider == &slider_referenceCrossfeedGainRight1)
    {
        processor.setReferenceCrossfeedGainRight1 (slider_referenceCrossfeedGainRight1.getValue());
        currChannel = Channel::RIGHT;
    }
    else if (slider == &slider_referenceCrossfeedGainLeft2)
    {
        processor.setReferenceCrossfeedGainLeft2 (slider_referenceCrossfeedGainLeft2.getValue());
        currChannel = Channel::LEFT;
    }
    else if (slider == &slider_referenceCrossfeedGainRight2)
    {
        processor.setReferenceCrossfeedGainRight2 (slider_referenceCrossfeedGainRight2.getValue());
        currChannel = Channel::RIGHT;
    }
    else if (slider == &slider_delayInMs)
    {
        processor.setCrossfeedDelayInMs (slider_delayInMs.getValue());
    }
    
    processor.updateCalibratingEQNode (EQNode (0, 0, 0), Channel::CENTER, "");
}

void ReferenceCalibrationPage::sliderDragStarted (juce::Slider *slider)
{
    processor.startCalibratingEQNode (EQNode (0, 0, 0), Channel::CENTER, "");
}

void ReferenceCalibrationPage::sliderDragEnded (juce::Slider *slider)
{
    processor.endCalibratingEQNode();
}
