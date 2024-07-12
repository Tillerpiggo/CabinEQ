/*
  ==============================================================================

    BinaryClassificationPage.cpp
    Created: 9 Jul 2024 4:25:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BinaryClassificationPage.h"

BinaryClassificationPage::BinaryClassificationPage(StartupMVPAudioProcessor& p)
    : audioProcessor(p)//, curveComponent(p.getCurve())
{
    addComponents();
    addListeners();
//    
    startTimer (10);
}

BinaryClassificationPage::~BinaryClassificationPage()
{
    removeListeners();
    stopTimer();
}



void BinaryClassificationPage::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
//    int curveComponentHeight = 200;
//
//    // Reserve the top 200 points for the curve component
//    curveComponent.setBounds(area.removeFromTop(curveComponentHeight).reduced(padding));

    // Calculate the height for the switch and bypass button
    int switchHeight = 30;
    int buttonHeight = 80; // Making both buttons taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - buttonHeight - (3 * padding);

    // Calculate the new height for the lower and higher preferred buttons (2/3 of available height)
    int buttonSectionHeight = (2 * availableHeight) / 3;
    int theSameButtonHeight = availableHeight / 3;

    // Calculate the width for each button
    int buttonWidth = (area.getWidth() - (3 * padding)) / 2;

    // Layout the lower and higher preferred buttons side by side, taking up 2/3 of available height
    auto buttonsArea = area.removeFromTop(buttonSectionHeight).reduced(padding);
    auto lowerButtonBounds = buttonsArea.removeFromLeft(buttonWidth).reduced(padding / 2);
    lowerPreferredButton.setBounds(lowerButtonBounds);
    auto higherButtonBounds = buttonsArea.reduced(padding / 2);
    higherPreferredButton.setBounds(higherButtonBounds);

    // Remove the green circle area and move up the toggle calibration button
    toggleCalibrationButton.setBounds(area.removeFromTop(switchHeight).reduced(padding / 2));

    // Calculate the area for the bypass and apply curve buttons, but only on the right half
    auto bottomButtonArea = area.removeFromBottom(buttonHeight).reduced(padding);
    auto rightHalfArea = bottomButtonArea.removeFromRight(bottomButtonArea.getWidth() / 2);
    int buttonSpacing = 5; // Small spacing between buttons

    // Calculate bounds for applyCurveButton and bypassButton
    auto applyCurveButtonArea = rightHalfArea.removeFromLeft((rightHalfArea.getWidth() / 2) - (buttonSpacing / 2));
    auto bypassButtonArea = rightHalfArea.reduced(buttonSpacing / 2);

    // Calculate the area for the referenceSlider to the left of the applyCurveButton
    auto referenceSliderArea = bottomButtonArea.removeFromLeft((bottomButtonArea.getWidth()) - (buttonSpacing / 2)).reduced(padding);

    referenceSlider.setBounds(referenceSliderArea);
    applyCurveButton.setBounds(applyCurveButtonArea);
    bypassButton.setBounds(bypassButtonArea);
}

void BinaryClassificationPage::sliderValueChanged(juce::Slider* slider)
{
    std::cout << "slider value changed" << std::endl;
    if (slider == &referenceSlider)
    {
        //audioProcessor.setBypassVolume(slider->getValue());
    }
}

void BinaryClassificationPage::buttonClicked(juce::Button* button)
{
    if (button == &toggleCalibrationButton)
    {
        //audioProcessor.toggleCalibration();

        bool isEnabled = toggleCalibrationButton.getToggleState();
        lowerPreferredButton.setEnabled (isEnabled);
        higherPreferredButton.setEnabled (isEnabled);
    }
    else if (button == lowerPreferredButton.getButtonPointer())
    {
        //audioProcessor.calibrateWith(CalibrationChoice::LowerPreferred);
    }
    else if (button == higherPreferredButton.getButtonPointer())
    {
        //audioProcessor.calibrateWith(CalibrationChoice::HigherPreferred);
    }
    else if (button == &bypassButton)
    {
        //audioProcessor.toggleBypass();
    }
    else if (button == &applyCurveButton)
    {
        //audioProcessor.applyCurve();
    }
}

void BinaryClassificationPage::addComponents()
{
    //addAndMakeVisible(curveComponent);
    addAndMakeVisible(lowerPreferredButton);
    addAndMakeVisible(higherPreferredButton);
    addAndMakeVisible(toggleCalibrationButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(applyCurveButton);
    addAndMakeVisible(referenceSlider);

    referenceSlider.setRange(-12.0f, 12.0f);
    
    lowerPreferredButton.setEnabled(false);
    higherPreferredButton.setEnabled(false);
}

void BinaryClassificationPage::addListeners()
{
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);
    applyCurveButton.addListener (this);
    referenceSlider.addListener (this);
}

void BinaryClassificationPage::removeListeners()
{
    toggleCalibrationButton.removeListener (this);
    lowerPreferredButton.removeListener();
    higherPreferredButton.removeListener();
    bypassButton.removeListener (this);
    applyCurveButton.removeListener (this);
    referenceSlider.removeListener (this);
}

void BinaryClassificationPage::timerCallback()
{
    /*
    // Update button text to match question
    Question currentQuestion = audioProcessor.getCurrentQuestion();
    lowerPreferredButton.setLabelText(currentQuestion.lowerText());
    higherPreferredButton.setLabelText(currentQuestion.higherText());

    // Update lights to match which tone is playing
    if (!toggleCalibrationButton.getToggleState())
    {
        // do nothing
    }
    else if (audioProcessor.isPlayingFirstNote())
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            lowerPreferredButton.setDeepRed (false);
            higherPreferredButton.setDeepRed (false);
        }
        else
        {
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (false);
        }
    }
    else
    {
        if (currentQuestion.getType() == QuestionType::Pan)
        {
            lowerPreferredButton.setDeepRed (true);
            higherPreferredButton.setDeepRed (true);
        }
        else
        {
            lowerPreferredButton.setDeepRed (false);
            higherPreferredButton.setDeepRed (true);
        }
    }
     */
}
