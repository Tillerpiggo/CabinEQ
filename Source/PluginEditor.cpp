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
    
    filterPage = std::make_unique<FilterPage> (p);
//    headphoneEQPage = std::make_unique<CabinEQPage> (p, "HeadphoneEQ");
//    speakerEQPage = std::make_unique<CabinEQPage> (p, "SpeakerEQ");
    
    tabbedComponent.addTab ("Filter", juce::Colours::lightgrey, filterPage.get(), false);
//    tabbedComponent.addTab ("HeadphoneEQ", juce::Colours::lightgrey, headphoneEQPage.get(), false);
//    tabbedComponent.addTab ("SpeakerEQ", juce::Colours::lightgrey, speakerEQPage.get(), false);

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
