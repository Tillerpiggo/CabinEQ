/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SetPointManager.h"


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
    prevButton.addListener (this);
    nextButton.addListener (this);
    bypassButton.addListener (this);
    
    resized(); // to update UI to be correct
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    for (auto& sliderPair : sliderPairs)
    {
        sliderPair->removeSliderListener (&audioProcessor);
    }
    
    prevButton.removeListener (this);
    nextButton.removeListener (this);
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
    curveComponent.setBounds (getLocalBounds().withBottom (200));
    
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

    // Calculate position for the bypassButton
    const int bypassButtonY = buttonY + buttonHeight + spacing; // Place it below the previous buttons

    // Center the bypassButton horizontally
    const int bypassButtonX = (getWidth() - buttonWidth) / 2;

    // Set bounds for the bypassButton
    bypassButton.setBounds(bypassButtonX, bypassButtonY, buttonWidth, buttonHeight);

    tabbedComponent.setBounds (getLocalBounds().withTop (200).withBottom (400));
}

// Creates a pair of sliders - one for gain, and one for balance
void StartupMVPAudioProcessorEditor::addSliderPair (int i)
{
    std::string idx = std::to_string(i);

    auto sliderPair = std::make_unique<SliderPair>(
        "Gain Slider " + idx, "Balance Slider " + idx,
        "gain_" + idx, "balance_" + idx,
        audioProcessor.parameters, i);
    sliderPair->addSliderListener (&audioProcessor);

    tabbedComponent.addTab("Set Point " + idx, juce::Colours::transparentWhite, sliderPair.get(), true);
    sliderPairs.push_back(std::move(sliderPair));
}

//// Creates a slider and label with the given name, for the given parameter, at the given index,
//// and connects that slider to the parameter via attachment
//juce::Slider& StartupMVPAudioProcessorEditor::addSlider (std::string name, std::string paramName, int idx)
//{
//    // Create the slider and label
//    auto slider = std::make_unique<juce::Slider>();
//    auto label = std::make_unique<juce::Label>();
//    
//    // Configure the slider and label
//    slider->setSliderStyle (juce::Slider::LinearHorizontal);
//    slider->setRange (-24.f, 48.f, 0.1f);
//    slider->getProperties().set ("index", idx);
//    slider->addListener (&audioProcessor);
//    label->setText (name, juce::dontSendNotification);
//    label->attachToComponent (slider.get(), true);
//    
//    sliders.push_back (std::move (slider));
//
//    // Create the slider attachment
//    auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
//                                                                                             audioProcessor.parameters, paramName, *slider);
//    sliderAttachments.push_back (std::move (attachment));
//    
//    return *sliders.back();
//}

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
