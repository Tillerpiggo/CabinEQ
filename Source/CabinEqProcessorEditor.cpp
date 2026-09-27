/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "CabinEqProcessorEditor.h"

CabinEqProcessorEditor::CabinEqProcessorEditor (CabinEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (genericEditor);
    setSize (500, 300);
}

CabinEqProcessorEditor::~CabinEqProcessorEditor()
{
}

//==============================================================================
void CabinEqProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void CabinEqProcessorEditor::resized()
{
    genericEditor.setBounds (getLocalBounds());
}
