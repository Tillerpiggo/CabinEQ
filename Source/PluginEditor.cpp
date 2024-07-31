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
    eqProfilePage = std::make_unique<EQProfilePage> (p);
    cabinEQPage = std::make_unique<CabinEQPage> (p);
    stupidExperimentPage = std::make_unique<StupidExperimentPage> (p);
    
    tabbedComponent.addTab ("Filter", juce::Colours::lightgrey, filterPage.get(), false);
    tabbedComponent.addTab ("Profile", juce::Colours::lightgrey, eqProfilePage.get(), false);
    tabbedComponent.addTab ("CabinEQ", juce::Colours::lightgrey, cabinEQPage.get(), false);
    tabbedComponent.addTab ("StupidExperimentPage", juce::Colours::lightgrey, stupidExperimentPage.get(), false);

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
