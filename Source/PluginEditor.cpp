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
    cabinEQPage = std::make_unique<CabinEQPage> (p);
    referencePage = std::make_unique<ReferencePage> (p);
//    spatialCalibrationPage = std::make_unique<SpatialCalibrationPage> (p);
    
    tabbedComponent.addTab ("Filter", juce::Colours::lightgrey, filterPage.get(), false);
    tabbedComponent.addTab ("CabinEQ", juce::Colours::lightgrey, cabinEQPage.get(), false);
    tabbedComponent.addTab ("Reference", juce::Colours::lightgrey, referencePage.get(), false);
//    tabbedComponent.addTab ("Spatial", juce::Colours::lightgrey, spatialCalibrationPage.get(), false);

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
