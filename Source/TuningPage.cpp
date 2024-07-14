/*
  ==============================================================================

    TuningPage.cpp
    Created: 12 Jul 2024 12:25:29am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "TuningPage.h"

TuningPage::TuningPage(StartupMVPAudioProcessor& p) : processor (p)
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
        sliderAttachments[i] = std::make_unique<juce::ParameterAttachment>(*processor.parameters.getParameter ("gain_" + std::to_string (i)),
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

TuningPage::~TuningPage()
{
    for (auto& button : plusButtons) button.removeListener(this);
    for (auto& button : minusButtons) button.removeListener(this);
    toggleCalibrationButton.removeListener (this);
    
    stopTimer();
}

void TuningPage::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColor);

    int circleRadius = 20;
    int spacing = getWidth() / 6;
    int x = spacing;
    int y = getHeight() / 3 - circleRadius; // Adjust Y position to be above the buttons

    for (int i = 0; i < circles.size(); ++i)
    {
        circles[i] = juce::Rectangle<float>(x - circleRadius, y - circleRadius, circleRadius * 2, circleRadius * 2);
        g.setColour((litUpIndex == i) ? circleColorOn : circleColorOff);
        g.fillEllipse(circles[i]);
        
        x += spacing;
    }
}

void TuningPage::resized()
{
    int buttonWidth = 50; // Adjust the width to a reasonable value
    int buttonHeight = 30; // Adjust the height of the buttons as needed
    int spacing = getWidth() / 6;
    int x = spacing;
    int y = getHeight() / 2 - 50; // Position buttons below the circles

    for (int i = 0; i < plusButtons.size(); ++i)
    {
        plusButtons[i].setBounds(x - buttonWidth / 2, y, buttonWidth, buttonHeight);
        minusButtons[i].setBounds(x - buttonWidth / 2, y + buttonHeight + 10, buttonWidth, buttonHeight);
        x += spacing;
    }

    // Position toggleCalibrationButton and nextButton below the buttons
    int controlButtonHeight = 30; // Adjust the height of the buttons as needed
    int controlButtonWidth = 100; // Adjust the width of the buttons as needed
    int controlButtonSpacing = 20; // Space between the buttons

    int controlButtonX = (getWidth() - (2 * controlButtonWidth + controlButtonSpacing)) / 2; // Center both buttons horizontally
    int controlButtonY = y + 2 * buttonHeight + 20; // Place the buttons below the plus/minus buttons with some margin

    toggleCalibrationButton.setBounds(controlButtonX, controlButtonY, controlButtonWidth, controlButtonHeight);
    nextButton.setBounds(controlButtonX + controlButtonWidth + controlButtonSpacing, controlButtonY, controlButtonWidth, controlButtonHeight);
}

void TuningPage::setCircleLitUp(int index)
{
    if (index >= -1 && index < circles.size())
    {
        litUpIndex = index;
        repaint();
    }
}

void TuningPage::sliderDragStarted (juce::Slider* /*slider*/)
{
    // No sliders, so no implementation needed
}

void TuningPage::sliderDragEnded (juce::Slider* /*slider*/)
{
    // No sliders, so no implementation needed
}

void TuningPage::sliderValueChanged (juce::Slider* /*slider*/)
{
    // No sliders, so no implementation needed
}

void TuningPage::buttonClicked (juce::Button* button)
{
    for (int i = 0; i < plusButtons.size(); ++i)
    {
        if (button == &plusButtons[i])
        {
            int tuningIdx = processor.getSliderCalibrationManager().getTuningIndices()[i];
            float newValue = processor.parameters.getParameterAsValue("gain_" + std::to_string(tuningIdx)).getValue();
            newValue += 1.0f; // Increment by 1 dB
            sliderAttachments[tuningIdx]->setValueAsPartOfGesture (newValue);
            processor.getSliderCalibrationManager().setAmplitudeAtIdx (tuningIdx, newValue);
            return;
        }
        else if (button == &minusButtons[i])
        {
            int tuningIdx = processor.getSliderCalibrationManager().getTuningIndices()[i];
            float newValue = processor.parameters.getParameterAsValue("gain_" + std::to_string(tuningIdx)).getValue();
            newValue -= 1.0f; // Decrement by 1 dB
            sliderAttachments[tuningIdx]->setValueAsPartOfGesture (newValue);
            processor.getSliderCalibrationManager().setAmplitudeAtIdx (tuningIdx, newValue);
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

void TuningPage::timerCallback()
{
    int idx = processor.getSliderCalibrationManager().getCurrentlyPlayingTuningIdx();
    setCircleLitUp (idx);
}

void TuningPage::parameterChangedCallback (float newValue, int tuningIdx)
{
    // Update logic if needed
}

int TuningPage::sliderIndexForTuningIndex (int tuningIdx)
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
