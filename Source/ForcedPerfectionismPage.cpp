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
    
    // Create parameter attachments
    for (int i = 0; i < parameterAttachments.size(); ++i)
    {
        parameterAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("gain_" + std::to_string (i)),
                                                                           [this, i](float newValue) { parameterChangedCallback(newValue, i); }, nullptr);
        parameterAttachments[i]->sendInitialUpdate();
    }
    
    startTimer (10);
    
    currIdx = processor.getSliderCalibrationManager().goToNextQuestion();
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
    
    stopTimer();
}

void ForcedPerfectionismPage::resized()
{
    auto area = getLocalBounds().reduced(20); // Adding padding around the whole UI
    area.removeFromTop (100);
    
    // Define height ratios for each section
    auto circleHeight = area.getHeight() / 4;
    auto sliderHeight = area.getHeight() / 8;
    auto buttonHeight = area.getHeight() / 8;
    auto lowerButtonHeight = area.getHeight() / 8;

    // Position circles
    auto circleArea = area.removeFromTop(circleHeight);
    auto circleWidth = circleArea.getWidth() / 2;
    auto circleDiameter = std::min(circleWidth, circleHeight) - 20; // Reduce diameter to bring circles closer

    circleOne = juce::Rectangle<float>(
        circleArea.getX() + (circleWidth - circleDiameter) / 2.0f - 10.0f,
        circleArea.getY() + (circleHeight - circleDiameter) / 2.0f,
        circleDiameter,
        circleDiameter
    );

    circleTwo = juce::Rectangle<float>(
        circleArea.getX() + circleWidth + (circleWidth - circleDiameter) / 2.0f + 10.0f,
        circleArea.getY() + (circleHeight - circleDiameter) / 2.0f,
        circleDiameter,
        circleDiameter
    );
    
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
    
    if (currPlayingCircle == 0) g.setColour (circleColorOn);
    else g.setColour (circleColorOff);
    g.fillEllipse(circleOne);
    
    if (currPlayingCircle == 1) g.setColour (circleColorOn);
    else g.setColour (circleColorOff);
    g.fillEllipse(circleTwo);
}

    
void ForcedPerfectionismPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->setValueAsPartOfGesture (slider->getValue());
    }
}

void ForcedPerfectionismPage::sliderDragStarted (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->beginGesture();
    }
}

void ForcedPerfectionismPage::sliderDragEnded (juce::Slider *slider)
{
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->endGesture();
    }
}
    
void ForcedPerfectionismPage::buttonClicked (juce::Button *button)
{
    if (button == &largePlusButton)
    {
        incrementCurrValueBy (3.0f);
    }
    else if (button == &smallPlusButton)
    {
        incrementCurrValueBy (0.1f);
    }
    else if (button == &smallMinusButton)
    {
        incrementCurrValueBy (0.1f);
    }
    else if (button == &largeMinusButton)
    {
        incrementCurrValueBy (3.0f);
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
        currIdx = processor.getSliderCalibrationManager().goToNextQuestion();
    }
}
    
void ForcedPerfectionismPage::timerCallback()
{
    int lastPlayingCircle = currPlayingCircle;
    if (processor.getSliderCalibrationManager().getCurrentlyPlayingIdx() == currIdx) currPlayingCircle = 0;
    else currPlayingCircle = 1;
    
    if (lastPlayingCircle != currPlayingCircle) repaint();
}

void ForcedPerfectionismPage::parameterChangedCallback (float newValue, int idx)
{
    if (idx == currIdx)
    {
        slider.setValue (newValue);
    }
}

void ForcedPerfectionismPage::incrementCurrValueBy (float increment)
{
    float newValue = processor.parameters.getParameterAsValue("gain_" + std::to_string(currIdx)).getValue();
    newValue += increment;
    parameterAttachments[currIdx]->setValueAsPartOfGesture (newValue);
    processor.getSliderCalibrationManager().setAmplitudeAtIdx (currIdx, newValue);
}
