/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor (StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), tabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtBottom),
      curveComponent (p.getCurve())
{
    setSize (800, 600);

    // Create a slider for each set point
    for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
    {
        addSliderPair (i);
    }
    
    // Add curve and container for sliders
    addAndMakeVisible (curveComponent);
    addAndMakeVisible (tabbedComponent);
    
    // Add buttons
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (bypassBalanceButton);
    prevButton.addListener (this);
    nextButton.addListener (this);
    bypassButton.addListener (this);
    bypassBalanceButton.addListener (this);
    
    resized(); // to update UI to be correct
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    for (auto& sliderPair : sliderGroups)
    {
        sliderPair->removeSliderListener (&audioProcessor);
    }
    
    prevButton.removeListener (this);
    nextButton.removeListener (this);
    bypassButton.removeListener (this);
    bypassBalanceButton.removeListener (this);
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
    
    // Add the PREV and NEXT buttons
    const int buttonHeight = 40; // Height of the buttons
    const int spacing = 20;      // Spacing between the buttons
    const int bottomMargin = 80; // Margin from the bottom of the view

    // Calculate the width and position of each button
    const int buttonWidth = (getWidth() - spacing) / 2;
    const int buttonY = getHeight() - buttonHeight - bottomMargin;

    // Center the buttons horizontally
    const int buttonAX = (getWidth() / 2) - buttonWidth - (spacing / 2);
    const int buttonBX = (getWidth() / 2) + (spacing / 2);

    // Set bounds for the buttons
    prevButton.setBounds(buttonAX, buttonY, buttonWidth, buttonHeight);
    nextButton.setBounds(buttonBX, buttonY, buttonWidth, buttonHeight);

    // Constants for button dimensions and spacing
    const int bypassButtonWidth = 100;
    const int bypassButtonHeight = 30;
    const int bypassSpacing = 10;

    // Calculate position for the first button (bypassButton)
    const int bypassButtonY = getHeight() - bypassButtonHeight - bypassSpacing;
    const int halfButtonWidth = (bypassButtonWidth + bypassSpacing) / 2;

    // Center the buttons horizontally
    const int bypassButtonX = (getWidth() / 2) - halfButtonWidth;
    const int balanceBypassButtonX = bypassButtonX + bypassButtonWidth + spacing;

    // Set bounds for the bypassButton
    bypassButton.setBounds(bypassButtonX, bypassButtonY, bypassButtonWidth, bypassButtonHeight);

    // Set bounds for the balanceBypassButton
    bypassBalanceButton.setBounds(balanceBypassButtonX, bypassButtonY, bypassButtonWidth, bypassButtonHeight);

    // Set bounds for the tabbedComponent
    tabbedComponent.setBounds(getLocalBounds().withTop(100).withBottom(400));
}

// Creates a pair of sliders - one for gain, and one for balance
void StartupMVPAudioProcessorEditor::addSliderPair (int i)
{
    std::string idx = std::to_string(i);

    auto sliderGroup = std::make_unique<SliderGroup> (audioProcessor.parameters, i);
    sliderGroup->addSliderListener (&audioProcessor);

    tabbedComponent.addTab("Set Point " + idx, juce::Colours::transparentWhite, sliderGroup.get(), true);
    sliderGroups.push_back(std::move(sliderGroup));
}

void StartupMVPAudioProcessorEditor::buttonClicked (juce::Button *button)
{
    if (button == &prevButton)
    {
        int idx = audioProcessor.goToPrevInterval();
        tabbedComponent.setCurrentTabIndex (idx);
    }
    else if (button == &nextButton)
    {
        int idx = audioProcessor.goToNextInterval();
        tabbedComponent.setCurrentTabIndex (idx);
    }
    else if (button == &bypassButton)
    {
        audioProcessor.toggleBypass();
    }
}
