/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

CabinEQProcessorEditor::CabinEQProcessorEditor(CabinEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), cabinEQPage (p)
{
    setSize(800, 620);
    addAndMakeVisible (cabinEQPage);
}

CabinEQProcessorEditor::~CabinEQProcessorEditor()
{
    // The binaryClassificationPage will be automatically deleted as it is owned by the tabbedComponent
}

//==============================================================================
void CabinEQProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void CabinEQProcessorEditor::resized()
{
    cabinEQPage.setBounds (getLocalBounds());
}
