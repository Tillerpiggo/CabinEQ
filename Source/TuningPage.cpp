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
    for (auto& slider : sliders)
    {
        slider.setRange(-24.0f, 48.0f, 0.01f);
        slider.setValue(0.0f);
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.addListener(this);
        addAndMakeVisible(slider);
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
    for (auto& slider : sliders) slider.removeListener(this);
    toggleCalibrationButton.removeListener (this);
    
    stopTimer();
}

void TuningPage::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColor);

    int circleRadius = 20;
    int spacing = getWidth() / 6;
    int x = spacing;
    int y = getHeight() / 3 - circleRadius; // Adjust Y position to be above the sliders

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
    int sliderHeight = getHeight() / 2; // Make sliders take up more vertical space
    int sliderWidth = 50; // Adjust the width to a reasonable value
    int x = getWidth() / 6;
    int y = getHeight() / 2 - 50; // Position sliders below the circles

    for (int i = 0; i < sliders.size(); ++i)
    {
        sliders[i].setBounds(x - sliderWidth / 2, y, sliderWidth, sliderHeight);
        x += getWidth() / 6;
    }

    // Position toggleCalibrationButton and nextButton below the sliders
    int buttonHeight = 30; // Adjust the height of the buttons as needed
    int buttonWidth = 100; // Adjust the width of the buttons as needed
    int buttonSpacing = 20; // Space between the buttons

    int buttonX = (getWidth() - (2 * buttonWidth + buttonSpacing)) / 2; // Center both buttons horizontally
    int buttonY = y + sliderHeight + 10; // Place the buttons below the sliders with some margin

    toggleCalibrationButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
    nextButton.setBounds(buttonX + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight);
}

void TuningPage::setCircleLitUp(int index)
{
    if (index >= -1 && index < circles.size())
    {
        litUpIndex = index;
        repaint();
    }
}

void TuningPage::sliderDragStarted (juce::Slider* slider)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        if (slider == &sliders[i])
        {
            sliderAttachments[i]->beginGesture();
            break;
        }
    }
}

void TuningPage::sliderDragEnded (juce::Slider* slider)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        if (slider == &sliders[i])
        {
            sliderAttachments[i]->endGesture();
            break;
        }
    }
}

void TuningPage::sliderValueChanged (juce::Slider* slider)
{
    for (int i = 0; i < sliders.size(); ++i)
    {
        if (slider == &sliders[i])
        {
            int tuningIdx = processor.getSliderCalibrationManager().getTuningIndices()[i];
            sliderAttachments[tuningIdx]->setValueAsPartOfGesture (slider->getValue());
            processor.getSliderCalibrationManager().setAmplitudeAtIdx (tuningIdx, slider->getValue());
            break;
        }
    }
}

void TuningPage::buttonClicked (juce::Button* button)
{
    if (button == &toggleCalibrationButton)
    {
        isCalibrating = ! isCalibrating;
        processor.getSliderCalibrationManager().setIsCalibrating (isCalibrating);
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
    int sliderIdx = sliderIndexForTuningIndex (tuningIdx);
    if (sliderIdx != -1) sliders[sliderIdx].setValue (newValue);
}

int TuningPage::sliderIndexForTuningIndex (int tuningIdx)
{
    const std::vector<float>& tuningIndices = processor.getSliderCalibrationManager().getTuningIndices();
    for (int i = 0; i < tuningIndices.size(); ++i)
    {
        if (tuningIndices[i] == tuningIdx)
        {
            return i;
        }
    }
    
    return -1;
}
