/*
  ==============================================================================

    BlindSliderPage.cpp
    Created: 17 Jul 2024 11:13:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BlindSliderPage.h"

BlindSliderPage::BlindSliderPage (StartupMVPAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (largePlusButton);
    addAndMakeVisible (smallPlusButton);
    addAndMakeVisible (smallMinusButton);
    addAndMakeVisible (largeMinusButton);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (progressLabel);
    
    largePlusButton.addListener (this);
    smallPlusButton.addListener (this);
    smallMinusButton.addListener (this);
    largeMinusButton.addListener (this);
    toggleCalibrationButton.addListener (this);
    nextButton.addListener (this);
    
    progressLabel.setText ("0%", juce::NotificationType::dontSendNotification);
    progressLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    progressLabel.setJustificationType (juce::Justification::centred);
    
    // Create parameter attachments
    for (int i = 0; i < parameterAttachments.size(); ++i)
    {
        parameterAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("gain_" + std::to_string (i)),
                                                                           [this, i](float newValue) { parameterChangedCallback(newValue, i); }, nullptr);
    }
    
    currIdx = processor.getSliderCalibrationManager().goToNextBlindQuestion();
}

BlindSliderPage::~BlindSliderPage()
{
    largePlusButton.removeListener (this);
    smallPlusButton.removeListener (this);
    smallMinusButton.removeListener (this);
    largeMinusButton.removeListener (this);
    toggleCalibrationButton.removeListener (this);
    nextButton.removeListener (this);
}

void BlindSliderPage::resized()
{
    auto area = getLocalBounds().reduced(20); // Adding padding around the whole UI
    area.removeFromTop (100);
    
    // Define height ratios for each section
    auto buttonHeight = area.getHeight() / 8;
    auto lowerButtonHeight = area.getHeight() / 8;
    auto labelHeight = area.getHeight() / 8;
    
    auto buttonArea = area.removeFromTop(buttonHeight);
    auto buttonWidth = buttonArea.getWidth() / 4;
    largeMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    smallMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    smallPlusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    largePlusButton.setBounds(buttonArea.reduced(5));

    auto lowerButtonArea = area.removeFromTop(lowerButtonHeight);
    auto lowerButtonWidth = lowerButtonArea.getWidth() / 2;
    toggleCalibrationButton.setBounds(lowerButtonArea.removeFromLeft(lowerButtonWidth).reduced(5));
    nextButton.setBounds(lowerButtonArea.reduced(5));

    progressLabel.setBounds(area.removeFromTop(labelHeight).reduced(10));
}

void BlindSliderPage::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColor);
}
    
void BlindSliderPage::buttonClicked (juce::Button *button)
{
    if (button == &largePlusButton)
    {
        incrementCurrValueBy (1.0f);
    }
    else if (button == &smallPlusButton)
    {
        incrementCurrValueBy (0.05f);
    }
    else if (button == &smallMinusButton)
    {
        incrementCurrValueBy (-0.05f);
    }
    else if (button == &largeMinusButton)
    {
        incrementCurrValueBy (-1.0f);
    }
    else if (button == &toggleCalibrationButton)
    {
        isCalibrating = ! isCalibrating;
        processor.getSliderCalibrationManager().setIsCalibrating (isCalibrating);
        
        if (isCalibrating && currIdx == -1) {
//            currIdx = processor.getSliderCalibrationManager().goToNextQuestion (-28.0f);
            parameterAttachments[currIdx]->sendInitialUpdate();
        }
        
        
        if (isCalibrating)
        {
            toggleCalibrationButton.setButtonText ("Stop Calibrating");
        }
        else
        {
            toggleCalibrationButton.setButtonText ("Start Calibrating");
        }
    }
    else if (button == &nextButton)
    {
        currIdx = processor.getSliderCalibrationManager().goToNextBlindQuestion();
        
        parameterAttachments[currIdx]->sendInitialUpdate();
//        progressLabel.setText (std::to_string (processor.getSliderCalibrationManager().getNumLockedIn()) + "%", juce::NotificationType::dontSendNotification);
    }
}

void BlindSliderPage::parameterChangedCallback (float newValue, int idx)
{
    // Don't really need to do anything, no UI to update
}

void BlindSliderPage::incrementCurrValueBy (float increment)
{
    if (currIdx == -1) return;
    
    float newValue = processor.parameters.getParameterAsValue("gain_" + std::to_string(currIdx)).getValue();
    newValue += increment;
    parameterAttachments[currIdx]->setValueAsPartOfGesture (newValue);
    processor.getSliderCalibrationManager().setAmplitudeAtIdx(currIdx, newValue);
}

