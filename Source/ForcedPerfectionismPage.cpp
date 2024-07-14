/*
  ==============================================================================

    ForcedPerfectionismPage.cpp
    Created: 14 Jul 2024 11:14:06am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ForcedPerfectionismPage.h"

ForcedPerfectionismPage::ForcedPerfectionismPage (StartupMVPAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (slider);
    addAndMakeVisible (largePlusButton);
    addAndMakeVisible (smallPlusButton);
    addAndMakeVisible (smallMinusButton);
    addAndMakeVisible (largeMinusButton);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (nextButton);
    
    slider.addListener (this);
    largePlusButton.addListener (this);
    smallPlusButton.addListener (this);
    smallMinusButton.addListener (this);
    largeMinusButton.addListener (this);
    toggleCalibrationButton.addListener (this);
    nextButton.addListener (this);
}

ForcedPerfectionismPage::~ForcedPerfectionismPage()
{
    slider.removeListener (this);
    largePlusButton.removeListener (this);
    smallPlusButton.removeListener (this);
    smallMinusButton.removeListener (this);
    largeMinusButton.removeListener (this);
    toggleCalibrationButton.removeListener (this);
    nextButton.removeListener (this);
}

void ForcedPerfectionismPage::resized()
{
    auto area = getLocalBounds();
    
    // Define height ratios for each section
    auto circleHeight = area.getHeight() / 4;
    auto sliderHeight = area.getHeight() / 8;
    auto buttonHeight = area.getHeight() / 8;
    auto lowerButtonHeight = area.getHeight() / 8;

    // Position circles
    auto circleArea = area.removeFromTop(circleHeight);
    auto circleWidth = circleArea.getWidth() / 2;
    circleOne = circleArea.removeFromLeft(circleWidth).reduced(10).toFloat();
    circleTwo = circleArea.reduced(10).toFloat();
    
    // Position slider
    slider.setBounds(area.removeFromTop(sliderHeight).reduced(10));
    
    // Position buttons (largeMinus, smallMinus, smallPlus, largePlus)
    auto buttonArea = area.removeFromTop(buttonHeight);
    auto buttonWidth = buttonArea.getWidth() / 4;
    largeMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    smallMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    smallPlusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    largePlusButton.setBounds(buttonArea.reduced(5));
    
    // Position buttons (toggleCalibrationButton, nextButton)
    auto lowerButtonArea = area.removeFromTop(lowerButtonHeight);
    auto lowerButtonWidth = lowerButtonArea.getWidth() / 2;
    toggleCalibrationButton.setBounds(lowerButtonArea.removeFromLeft(lowerButtonWidth).reduced(5));
    nextButton.setBounds(lowerButtonArea.reduced(5));
}

void ForcedPerfectionismPage::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColor);
    
    g.setColour(circleColorOff);
    g.fillEllipse(circleOne);
    
    g.setColour(circleColorOn);
    g.fillEllipse(circleTwo);
}

    
void ForcedPerfectionismPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        
    }
}

void ForcedPerfectionismPage::sliderDragStarted (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        
    }
}

void ForcedPerfectionismPage::sliderDragEnded (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        
    }
}
    
void ForcedPerfectionismPage::buttonClicked (juce::Button *button)
{
    if (button == &largePlusButton)
    {
        
    }
    else if (button == &smallPlusButton)
    {
        
    }
    else if (button == &smallMinusButton)
    {
        
    }
    else if (button == &largeMinusButton)
    {
        
    }
    else if (button == &toggleCalibrationButton)
    {
        
    }
    else if (button == &nextButton)
    {
        
    }
}
    
void ForcedPerfectionismPage::timerCallback()
{
    // update circles
}
