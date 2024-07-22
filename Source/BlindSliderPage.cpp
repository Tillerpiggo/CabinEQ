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
    addAndMakeVisible (tinyPlusButton);
    addAndMakeVisible (teenyTinyPlusButton);
    addAndMakeVisible (teenyTinyMinusButton);
    addAndMakeVisible (tinyMinusButton);
    addAndMakeVisible (smallMinusButton);
    addAndMakeVisible (largeMinusButton);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (progressLabel);
    
    largePlusButton.addListener (this);
    smallPlusButton.addListener (this);
    tinyPlusButton.addListener (this);
    teenyTinyPlusButton.addListener (this);
    teenyTinyMinusButton.addListener (this);
    tinyMinusButton.addListener (this);
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
    
    currIdx = 0;//processor.getSliderCalibrationManager().goToNextBlindQuestion();
}

BlindSliderPage::~BlindSliderPage()
{
    largePlusButton.removeListener (this);
    smallPlusButton.removeListener (this);
    tinyPlusButton.removeListener (this);
    teenyTinyPlusButton.removeListener (this);
    teenyTinyMinusButton.removeListener (this);
    tinyMinusButton.removeListener (this);
    smallMinusButton.removeListener (this);
    largeMinusButton.removeListener (this);
    toggleCalibrationButton.removeListener (this);
    nextButton.removeListener (this);
}

void BlindSliderPage::resized()
{
    auto area = getLocalBounds().reduced(20); // Adding padding around the whole UI
    area.removeFromTop(100);
    
    // Define height ratios for each section
    auto buttonHeight = area.getHeight() / 8;
    auto lowerButtonHeight = area.getHeight() / 8;
    auto labelHeight = area.getHeight() / 8;
    
    auto buttonArea = area.removeFromTop(buttonHeight);
    auto buttonWidth = buttonArea.getWidth() / 8; // Adjusted for eight buttons now
    largeMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    smallMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    tinyMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    teenyTinyMinusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    teenyTinyPlusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
    tinyPlusButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(5));
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

void BlindSliderPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        int idxToUpdate = processor.getSliderCalibrationManager().setAmplitudeAtBlindIdx (slider->getValue());
        parameterAttachments[idxToUpdate]->setValueAsPartOfGesture (slider->getValue());
    }
}

void BlindSliderPage::sliderDragStarted (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        processor.getSliderCalibrationManager().playBlindInterval();
    }
}

void BlindSliderPage::sliderDragEnded (juce::Slider *slider)
{
    // Do nothing for now
}
    
void BlindSliderPage::buttonClicked (juce::Button *button)
{
    if (button == &largePlusButton)
    {
        incrementCurrValueBy (4.0f);
    }
    else if (button == &smallPlusButton)
    {
        incrementCurrValueBy (2.0f);
    }
    else if (button == &tinyPlusButton)
    {
        incrementCurrValueBy (1.0f);
    }
    else if (button == &teenyTinyPlusButton)
    {
        incrementCurrValueBy (0.5f);
    }
    else if (button == &teenyTinyMinusButton)
    {
        incrementCurrValueBy (-0.5f);
    }
    else if (button == &tinyMinusButton)
    {
        incrementCurrValueBy (-1.0f);
    }
    else if (button == &smallMinusButton)
    {
        incrementCurrValueBy (-2.0f);
    }
    else if (button == &largeMinusButton)
    {
        incrementCurrValueBy (-4.0f);
    }
    else if (button == &toggleCalibrationButton)
    {
        isCalibrating = ! isCalibrating;
        processor.getSliderCalibrationManager().setIsCalibrating (isCalibrating);
        
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
        
//        parameterAttachments[currIdx]->sendInitialUpdate();
        progressLabel.setText (std::to_string (processor.getSliderCalibrationManager().getCurrBlindPercent()) + "%", juce::NotificationType::dontSendNotification);
    }
}

void BlindSliderPage::parameterChangedCallback (float newValue, int idx)
{
    // Don't really need to do anything, no UI to update
}

void BlindSliderPage::incrementCurrValueBy (float increment)
{
    if (currIdx == -1) return;
    
    auto [idxToUpdate, newValue] = processor.getSliderCalibrationManager().incrementAmplitudeAtBlindIdxAndPlay (increment);
    parameterAttachments[idxToUpdate]->setValueAsPartOfGesture (newValue);
}

