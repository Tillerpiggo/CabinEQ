/*
  ==============================================================================

    PanTuningPage.cpp
    Created: 13 Jul 2024 3:48:22pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PanTuningPage.h"

PanTuningPage::PanTuningPage(StartupMVPAudioProcessor& p) : processor (p)
{
    for (int i = 0; i < plusButtons.size(); ++i)
    {
        plusButtons[i].setButtonText("+");
        plusButtons[i].addListener(this);
        addAndMakeVisible(plusButtons[i]);
        
        minusButtons[i].setButtonText("-");
        minusButtons[i].addListener(this);
        addAndMakeVisible(minusButtons[i]);
    }
    
    for (int i = 0; i < sliderAttachments.size(); ++i)
    {
        sliderAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("pan_" + std::to_string (i)),
                                                                           [this, i](float newValue) { parameterChangedCallback(newValue, i); }, nullptr);
        sliderAttachments[i]->sendInitialUpdate();
    }
    
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (nextButton);
    
    toggleCalibrationButton.addListener (this);
    nextButton.addListener (this);

    setSize(600, 400);
    
    startTimer (10);
}

PanTuningPage::~PanTuningPage()
{
    for (auto& button : plusButtons) button.removeListener(this);
    for (auto& button : minusButtons) button.removeListener(this);
    toggleCalibrationButton.removeListener (this);
    
    stopTimer();
}

void PanTuningPage::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColor);

    int circleRadius = 20;
    int spacing = getHeight() / 6;
    int y = spacing;

    for (int i = 0; i < circles.size(); ++i)
    {
        circles[i] = juce::Rectangle<float>(getWidth() / 2 - circleRadius, y - circleRadius, circleRadius * 2, circleRadius * 2);
        g.setColour((litUpIndex == i) ? circleColorOn : circleColorOff);
        g.fillEllipse(circles[i]);
        
        y += spacing;
    }
}

void PanTuningPage::resized()
{
    int buttonWidth = 50; // Adjust the width to a reasonable value
    int buttonHeight = 30; // Adjust the height of the buttons as needed
    int spacing = getHeight() / 6;
    int y = spacing;

    for (int i = 0; i < plusButtons.size(); ++i)
    {
        minusButtons[i].setBounds(getWidth() / 2 - 100, y - buttonHeight / 2, buttonWidth, buttonHeight);
        plusButtons[i].setBounds(getWidth() / 2 + 50, y - buttonHeight / 2, buttonWidth, buttonHeight);
        y += spacing;
    }

    // Position toggleCalibrationButton and nextButton below the buttons
    int controlButtonHeight = 30; // Adjust the height of the buttons as needed
    int controlButtonWidth = 100; // Adjust the width of the buttons as needed
    int controlButtonSpacing = 20; // Space between the buttons

    int controlButtonX = (getWidth() - (2 * controlButtonWidth + controlButtonSpacing)) / 2; // Center both buttons horizontally
    int controlButtonY = y; // Place the buttons below the plus/minus buttons with some margin

    toggleCalibrationButton.setBounds(controlButtonX, controlButtonY, controlButtonWidth, controlButtonHeight);
    nextButton.setBounds(controlButtonX + controlButtonWidth + controlButtonSpacing, controlButtonY, controlButtonWidth, controlButtonHeight);
}

void PanTuningPage::setCircleLitUp(int index)
{
    if (index >= -1 && index < circles.size())
    {
        litUpIndex = index;
        repaint();
    }
}

void PanTuningPage::buttonClicked (juce::Button* button)
{
    for (int i = 0; i < plusButtons.size(); ++i)
    {
        if (button == &plusButtons[i])
        {
            int tuningIdx = processor.getSliderCalibrationManager().getTuningIndices()[i];
            float newValue = processor.parameters.getParameterAsValue("pan_" + std::to_string(tuningIdx)).getValue();
            newValue += 0.1f; // Increment pan by 0.1
            sliderAttachments[tuningIdx]->setValueAsPartOfGesture (newValue);
            processor.getSliderCalibrationManager().setPanAtIdx (tuningIdx, newValue);
            return;
        }
        else if (button == &minusButtons[i])
        {
            int tuningIdx = processor.getSliderCalibrationManager().getTuningIndices()[i];
            float newValue = processor.parameters.getParameterAsValue("pan_" + std::to_string(tuningIdx)).getValue();
            newValue -= 0.1f; // Decrement pan by 0.1
            sliderAttachments[tuningIdx]->setValueAsPartOfGesture (newValue);
            processor.getSliderCalibrationManager().setPanAtIdx (tuningIdx, newValue);
            return;
        }
    }

    if (button == &toggleCalibrationButton)
    {
        isCalibrating = ! isCalibrating;
        std::cout << "started tuning" << std::endl;
        processor.getSliderCalibrationManager().setIsCalibrating (isCalibrating);
        std::cout << "is calibrating!!" << std::endl;
    }
    else if (button == &nextButton)
    {
        std::cout << "next button pressed" << std::endl;
        processor.getSliderCalibrationManager().changeTuningIndices();
        for (int i = 0; i < sliderAttachments.size(); ++i) sliderAttachments[i]->sendInitialUpdate();
    }
}

void PanTuningPage::timerCallback()
{
    int idx = processor.getSliderCalibrationManager().getCurrentlyPlayingTuningIdx();
    setCircleLitUp (idx);
}

void PanTuningPage::parameterChangedCallback (float newValue, int tuningIdx)
{
    // Update logic if needed
}

int PanTuningPage::sliderIndexForTuningIndex (int tuningIdx)
{
    const std::vector<int>& tuningIndices = processor.getSliderCalibrationManager().getTuningIndices();
    for (int i = 0; i < tuningIndices.size(); ++i)
    {
        if (tuningIndices[i] == tuningIdx)
        {
            return i;
        }
    }
    
    return -1;
}