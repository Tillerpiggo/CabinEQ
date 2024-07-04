/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), curveComponent (p.getCurve())
{
    setSize (800, 620);
    
    addAndMakeVisible (curveComponent);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (lowerPreferredButton);
    addAndMakeVisible (higherPreferredButton);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (applyCurveButton);
    addAndMakeVisible (redCircle);
    addAndMakeVisible (blueCircle);
    
    redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
    blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);
    applyCurveButton.addListener (this);
    
    lowerPreferredButton.setEnabled (false);
    higherPreferredButton.setEnabled (false);

    resized(); // to update UI to be correct
    
    startTimer(16); // 60+ times/s
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    toggleCalibrationButton.removeListener (this);
    lowerPreferredButton.removeListener (this);
    higherPreferredButton.removeListener (this);
    bypassButton.removeListener (this);
    applyCurveButton.removeListener (this);
    
    setLookAndFeel(nullptr);
    
    stopTimer();
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int padding = 10;
    int curveComponentHeight = 200;

    // Reserve the top 200 points for the curve component
    curveComponent.setBounds(area.removeFromTop(curveComponentHeight).reduced(padding));

    // Calculate the height for the switch and bypass button
    int switchHeight = 30;
    int buttonHeight = 80; // Making both buttons taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - buttonHeight - (3 * padding);

    // Calculate the width for each button
    int buttonWidth = (area.getWidth() - (3 * padding)) / 2;

    // Layout the lower and higher preferred buttons side by side, taking up available height
    auto buttonsArea = area.removeFromTop(availableHeight).reduced(padding);
    auto lowerButtonBounds = buttonsArea.removeFromLeft(buttonWidth).reduced(padding / 2);
    lowerPreferredButton.setBounds(lowerButtonBounds);
    auto higherButtonBounds = buttonsArea.reduced(padding / 2);
    higherPreferredButton.setBounds(higherButtonBounds);

    // Place the calibration switch below the buttons
    toggleCalibrationButton.setBounds(area.removeFromTop(switchHeight).reduced(padding / 2));

    // Calculate the area for the bypass and apply curve buttons, but only on the right half
    auto bottomButtonArea = area.removeFromBottom(buttonHeight).reduced(padding);
    auto rightHalfArea = bottomButtonArea.removeFromRight(bottomButtonArea.getWidth() / 2);
    int buttonSpacing = 5; // Small spacing between buttons

    // Calculate bounds for applyCurveButton and bypassButton
    auto applyCurveButtonArea = rightHalfArea.removeFromLeft((rightHalfArea.getWidth() / 2) - (buttonSpacing / 2));
    auto bypassButtonArea = rightHalfArea.reduced(buttonSpacing / 2);

    applyCurveButton.setBounds(applyCurveButtonArea);
    bypassButton.setBounds(bypassButtonArea);

    // Calculate the size and position for the circles
    int circleDiameter = 20;
    int margin = 5;
    int circleSpacing = 4;
    int leftOffset = 3;

    // Calculate individual circle bounds
    auto redCircleBounds = getLocalBounds().removeFromBottom(circleDiameter + margin).removeFromLeft(circleDiameter + margin).withSizeKeepingCentre(circleDiameter, circleDiameter);
    redCircleBounds.translate(leftOffset + (margin / 2), -margin / 2);
    
    auto blueCircleBounds = redCircleBounds.translated(circleDiameter + circleSpacing, 0);

    redCircle.setBounds(redCircleBounds);
    blueCircle.setBounds(blueCircleBounds);
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &toggleCalibrationButton)
    {
        audioProcessor.toggleCalibration();
        
        bool isEnabled = toggleCalibrationButton.getToggleState();
        lowerPreferredButton.setEnabled (isEnabled);
        higherPreferredButton.setEnabled (isEnabled);
    }
    else if (button == &lowerPreferredButton)
    {
        audioProcessor.calibrateWith (CalibrationChoice::LowerPreferred);
    }
    else if (button == &higherPreferredButton)
    {
        audioProcessor.calibrateWith (CalibrationChoice::HigherPreferred);
    }
    else if (button == &bypassButton)
    {
        audioProcessor.toggleBypass();
    }
    else if (button == &applyCurveButton)
    {
        audioProcessor.applyCurve();
    }
}

void StartupMVPAudioProcessorEditor::timerCallback()
{
    // Update button text to match question
    Question currentQuestion = audioProcessor.getCurrentQuestion();
    lowerPreferredButton.setButtonText (currentQuestion.lowerText());
    higherPreferredButton.setButtonText (currentQuestion.higherText());
    
    // Update lights to match which tone is playing
    if (! toggleCalibrationButton.getToggleState())
    {
        redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
        blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    }
    else if (audioProcessor.isPlayingFirstNote())
    {
        redCircle.setColor(juce::Colours::lightcoral.withSaturation (1.0f));
        blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    }
    else
    {
        redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
        blueCircle.setColor(juce::Colours::lightskyblue.withSaturation (1.0f));
    }
    
}
