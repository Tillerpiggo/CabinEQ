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
    addAndMakeVisible (redCircle);
    addAndMakeVisible (blueCircle);
    
    redCircle.setColor(juce::Colours::lightcoral.withAlpha (0.7f));
    blueCircle.setColor(juce::Colours::lightskyblue.withAlpha (0.7f));
    
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);
    
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
    int bypassButtonHeight = 80; // Making the bypass button taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - bypassButtonHeight - (3 * padding);

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

    // Place the bypass button at the bottom right corner, thinner and taller
    auto bypassButtonArea = getLocalBounds().removeFromBottom(bypassButtonHeight).removeFromRight(buttonWidth / 2).reduced(padding);
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
