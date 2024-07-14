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
    sliderPage = std::make_unique<SliderPage> (p);
    leftRightPage = std::make_unique<LeftRightPage> (p);
    tuningPage = std::make_unique<TuningPage> (p);
    panTuningPage = std::make_unique<PanTuningPage> (p);
//    forcedPerfectionismPage = std::make_unique<ForcedPerfectionismPage> (p);
    
    tabbedComponent.addTab ("Filter", juce::Colours::lightgrey, filterPage.get(), false);
    tabbedComponent.addTab ("Sliders", juce::Colours::lightgrey, sliderPage.get(), false);
    tabbedComponent.addTab ("Left/Right", juce::Colours::lightgrey, leftRightPage.get(), false);
    tabbedComponent.addTab ("Tuning", juce::Colours::lightgrey, tuningPage.get(), false);
    tabbedComponent.addTab ("Left/Right Tuning", juce::Colours::lightgrey, panTuningPage.get(), false);
    tabbedComponent.addTab ("Forced Perfectionism", juce::Colours::lightgrey, forcedPerfectionismPage.get(), false);

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
