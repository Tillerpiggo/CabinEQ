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
    setSize (800, 600);

    // Add curve and container for sliders
    addAndMakeVisible (curveComponent);
    
    // Add buttons
    addAndMakeVisible (bypassButton);
    bypassButton.addListener (this);
    
    resized(); // to update UI to be correct
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    bypassButton.removeListener (this);
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    curveComponent.setBounds (getLocalBounds().withBottom (100));

    const int bypassButtonWidth = 100;
    const int bypassButtonHeight = 30;
    const int spacing = 10;

    // Calculate position for the first button (bypassButton)
    const int bypassButtonY = getHeight() - bypassButtonHeight - spacing;
    const int halfButtonWidth = (bypassButtonWidth + spacing) / 2;

    const int bypassButtonX = (getWidth() / 2) - halfButtonWidth;
    bypassButton.setBounds(bypassButtonX, bypassButtonY, bypassButtonWidth, bypassButtonHeight);
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &bypassButton)
    {
        audioProcessor.toggleBypass();
    }
}
