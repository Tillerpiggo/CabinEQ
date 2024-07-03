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
    setSize (800, 420);

    // Create and apply the custom look-and-feel
//    clearSoundLookAndFeel = std::make_unique<ClearSoundLookAndFeel>();
//    setLookAndFeel(clearSoundLookAndFeel.get());

    addAndMakeVisible (curveComponent);
    addAndMakeVisible (toggleCalibrationButton);
    addAndMakeVisible (lowerPreferredButton);
    addAndMakeVisible (higherPreferredButton);
    addAndMakeVisible (bypassButton);
    
    toggleCalibrationButton.addListener (this);
    lowerPreferredButton.addListener (this);
    higherPreferredButton.addListener (this);
    bypassButton.addListener (this);

    resized(); // to update UI to be correct
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    toggleCalibrationButton.removeListener (this);
    lowerPreferredButton.removeListener (this);
    higherPreferredButton.removeListener (this);
    bypassButton.removeListener (this);
    
    setLookAndFeel(nullptr);
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
    
    // Calculate the height for the switch and bypass button
    int switchHeight = 30;
    int bypassButtonHeight = 80; // Making the bypass button taller

    // Calculate the remaining height for the lower and higher preferred buttons
    int availableHeight = area.getHeight() - switchHeight - bypassButtonHeight - (3 * padding);

    // Calculate the width for each button
    int buttonWidth = (area.getWidth() - (3 * padding)) / 2;

    // Layout the lower and higher preferred buttons side by side, taking up available height
    auto buttonsArea = area.removeFromTop(availableHeight).reduced(padding);
    lowerPreferredButton.setBounds(buttonsArea.removeFromLeft(buttonWidth).reduced(padding / 2));
    higherPreferredButton.setBounds(buttonsArea.reduced(padding / 2));

    // Place the calibration switch below the buttons
    toggleCalibrationButton.setBounds(area.removeFromTop(switchHeight).reduced(padding / 2));

    // Place the bypass button at the bottom right corner, thinner and taller
    auto bypassButtonArea = getLocalBounds().removeFromBottom(bypassButtonHeight).removeFromRight(buttonWidth / 2).reduced(padding);
    bypassButton.setBounds(bypassButtonArea);
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &toggleCalibrationButton)
    {
        audioProcessor.toggleCalibration();
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
