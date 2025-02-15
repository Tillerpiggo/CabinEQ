/*
  ==============================================================================

    FreeTrialBanner.cpp
    Created: 18 Jan 2025 1:21:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "FreeTrialBanner.h"


FreeTrialBanner::FreeTrialBanner()
{
    secondsLeftUntilReset = resetCycleInSeconds;
    
    addAndMakeVisible (freeTrialLabel);
    addAndMakeVisible (timeLabel);
    addButton (&activateLicenseButton);
    freeTrialLabel.setText ("Free Trial - current profiles will lock in 2 hrs", juce::dontSendNotification);
    timeLabel.setText (convertSecondsToTimeFormat (resetCycleInSeconds), juce::dontSendNotification);
    timeLabel.setJustificationType (juce::Justification::right);
    
    addButtonAction (&activateLicenseButton, [this](juce::Button* button) {
        listener->showActivateLicenseForm();
    });
    
    startTimer (1000);
}
FreeTrialBanner::~FreeTrialBanner()
{
    stopTimer();
}

void FreeTrialBanner::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::goldenrod);
}

void FreeTrialBanner::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    layout.addRow ({ Space (&freeTrialLabel), Space(), Space(&activateLicenseButton) });
    layout.updateComponentBounds();
}

void FreeTrialBanner::setListener (FreeTrialListener* listener)
{
    this->listener = listener;
}

void FreeTrialBanner::timerCallback()
{
    secondsLeftUntilReset--;
    if (secondsLeftUntilReset < 0)
    {
        secondsLeftUntilReset = resetCycleInSeconds;
        if (listener != nullptr)
            listener->freeTrialDidReset();
    }
    
    freeTrialLabel.setText ("Free Trial - current profiles will lock in " +  convertSecondsToTimeFormat (secondsLeftUntilReset), juce::dontSendNotification);
//    timeLabel.setText (, juce::dontSendNotification);
}

std::string FreeTrialBanner::convertSecondsToTimeFormat(int totalSeconds) {
    // Calculate components
    int minutes = totalSeconds / 60;
    int seconds = (totalSeconds % 60);

    // Use a stringstream to format the time as a string
    std::ostringstream timeStream;
    timeStream << std::setw(2) << std::setfill('0') << minutes << ":"
               << std::setw(2) << std::setfill('0') << seconds;

    return timeStream.str(); // Return the formatted string
}
