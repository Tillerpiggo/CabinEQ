/*
  ==============================================================================

    BlindSliderPage.cpp
    Created: 17 Jul 2024 11:13:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#include "BlindSliderPage.h"

juce::Slider slider;
juce::TextButton calibrationToggleButton { "Start Calibration" };
juce::TextButton nextButton { "Next >" };
juce::Label progressLabel;

//===============================================
BlindSliderPage::BlindSliderPage (StartupMVPAudioProcessor& p) : processor (p) 
{
//    addAndMakeVisible (slider);
//    addAndMakeVisible (calibrationToggleButton);
//    addAndMakeVisible (nextButton);
//    addAndMakeVisible (progressLabel);
//    
//    slider.addListener (this);
//    calibrationToggleButton.addListener (this);
//    nextButton.addListener (this);
//    
//    progressLabel.setText ("0%", juce::NotificationType::dontSendNotification);
//    progressLabel.setColour (juce::Label::textColourId, juce::Colours::black);
//    progressLabel.setJustificationType (juce::Justification::centred);
//    
//    for (int i = 0; i < parameterAttachments.size(); ++i)
//    {
//        parameterAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("gain_" + std::to_string (i)),
//                                                                           [this, i](float newValue) { parameterChangedCallback(newValue, i); }, nullptr);
//    }
}

BlindSliderPage::~BlindSliderPage()
{
//    slider.removeListener (this);
//    calibrationToggleButton.removeListener (this);
//    nextButton.removeListener (this);
}

//===============================================
void BlindSliderPage::resized()
{
//    auto area = getLocalBounds();
//
//    auto sliderArea = area.removeFromTop(getHeight() / 3);
//    slider.setBounds(sliderArea.reduced(getWidth() / 10, 0));
//    
//    auto buttonArea = area.removeFromTop(getHeight() / 6);
//    auto buttonWidth = buttonArea.getWidth() / 2;
//    
//    calibrationToggleButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(10));
//    nextButton.setBounds(buttonArea.reduced(10));
//    progressLabel.setBounds(area.reduced(10));
}

void BlindSliderPage::paint (juce::Graphics& g)
{
//    g.fillAll (juce::Colours::white);
}

//===============================================
void BlindSliderPage::sliderValueChanged (juce::Slider *slider)
{
    
}

void BlindSliderPage::sliderDragStarted (juce::Slider *slider)
{
    
}

void BlindSliderPage::sliderDragEnded (juce::Slider *slider)
{
    
}

void BlindSliderPage::buttonClicked (juce::Button *button)
{
    
}

//===============================================
void BlindSliderPage::parameterChangedCallback (float newValue, int idx)
{
    
}
 */

#include "BlindSliderPage.h"

BlindSliderPage::BlindSliderPage (StartupMVPAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (slider);
    addAndMakeVisible (largePlusButton);
    addAndMakeVisible (smallPlusButton);
    addAndMakeVisible (smallMinusButton);
    addAndMakeVisible (largeMinusButton);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (numCompletedLabel);
    
    slider.setRange (-24.0f, 48.0f);
    slider.addListener (this);
    largePlusButton.addListener (this);
    smallPlusButton.addListener (this);
    smallMinusButton.addListener (this);
    largeMinusButton.addListener (this);
    toggleCalibrationButton.addListener (this);
    nextButton.addListener (this);
    
    numCompletedLabel.setText ("0/22", juce::NotificationType::dontSendNotification);
    numCompletedLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    numCompletedLabel.setJustificationType (juce::Justification::centred);
    
    // Create parameter attachments
    for (int i = 0; i < parameterAttachments.size(); ++i)
    {
        parameterAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("gain_" + std::to_string (i)),
                                                                           [this, i](float newValue) { parameterChangedCallback(newValue, i); }, nullptr);
        //parameterAttachments[i]->sendInitialUpdate();
    }
    
    startTimer (10);
    
    currIdx = -1;
    //parameterAttachments[currIdx]->sendInitialUpdate();
}

BlindSliderPage::~BlindSliderPage()
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

void BlindSliderPage::resized()
{
    auto area = getLocalBounds().reduced(20); // Adding padding around the whole UI
    area.removeFromTop (100);
    
    // Define height ratios for each section
    auto circleHeight = area.getHeight() / 4;
    auto sliderHeight = area.getHeight() / 8;
    auto buttonHeight = area.getHeight() / 8;
    auto lowerButtonHeight = area.getHeight() / 8;
    auto labelHeight = area.getHeight() / 8;

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

    // Position numCompletedLabel centered below the rest of the contents
    numCompletedLabel.setBounds(area.removeFromTop(labelHeight).reduced(10));
}

void BlindSliderPage::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColor);
    
    if (currPlayingCircle == 0) g.setColour (circleColorOn);
    else g.setColour (circleColorOff);
    g.fillEllipse(circleOne);
    
    if (currPlayingCircle == 1) g.setColour (circleColorOn);
    else g.setColour (circleColorOff);
    g.fillEllipse(circleTwo);
}

    
void BlindSliderPage::sliderValueChanged (juce::Slider *slider)
{
    if (currIdx == -1) return;
    
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->setValueAsPartOfGesture (slider->getValue());
    }
}

void BlindSliderPage::sliderDragStarted (juce::Slider *slider)
{
    if (currIdx == -1) return;
    
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->beginGesture();
    }
}

void BlindSliderPage::sliderDragEnded (juce::Slider *slider)
{
    if (currIdx == -1) return;
    
    if (slider == &this->slider)
    {
        parameterAttachments[currIdx]->endGesture();
    }
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
            currIdx = processor.getSliderCalibrationManager().goToNextQuestion (-28.0f);
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
        std::cout << "Going to next question from next button" << std::endl;
        currIdx = processor.getSliderCalibrationManager().goToNextQuestion (slider.getValue());
        parameterAttachments[currIdx]->sendInitialUpdate();
        numCompletedLabel.setText (std::to_string (processor.getSliderCalibrationManager().getNumLockedIn()) + "/22", juce::NotificationType::dontSendNotification);
    }
}
    
void BlindSliderPage::timerCallback()
{
    int lastPlayingCircle = currPlayingCircle;
    if (processor.getSliderCalibrationManager().getCurrentlyPlayingIdx() == currIdx) currPlayingCircle = 0;
    else currPlayingCircle = 1;
    
    if (lastPlayingCircle != currPlayingCircle) repaint();
}

void BlindSliderPage::parameterChangedCallback (float newValue, int idx)
{
    if (idx == currIdx)
    {
        slider.setValue (newValue);
    }
}

void BlindSliderPage::incrementCurrValueBy (float increment)
{
    if (currIdx == -1) return;
    
    float newValue = processor.parameters.getParameterAsValue("gain_" + std::to_string(currIdx)).getValue();
    newValue += increment;
    parameterAttachments[currIdx]->setValueAsPartOfGesture (newValue);
}

