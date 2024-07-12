/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

StartupMVPAudioProcessorEditor::StartupMVPAudioProcessorEditor(StartupMVPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      tabbedComponent (juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    setSize(800, 620);
    
    binaryClassificationPage = std::make_unique<BinaryClassificationPage> (p);
    //sliderPage = std::make_unique<SliderPage> (p);
//
    tabbedComponent.addTab ("Binary Classification", juce::Colours::lightgrey, binaryClassificationPage.get(), false);
    //tabbedComponent.addTab ("Sliders", juce::Colours::lightgrey, sliderPage.get(), false);

    addAndMakeVisible(tabbedComponent);
}

StartupMVPAudioProcessorEditor::~StartupMVPAudioProcessorEditor()
{
    // The binaryClassificationPage will be automatically deleted as it is owned by the tabbedComponent
}

//==============================================================================
void StartupMVPAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void StartupMVPAudioProcessorEditor::resized()
{
    tabbedComponent.setBounds (getLocalBounds());
}
