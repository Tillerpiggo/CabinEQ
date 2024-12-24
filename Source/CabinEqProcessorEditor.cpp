/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "CabinEqProcessorEditor.h"

CabinEqProcessorEditor::CabinEqProcessorEditor(CabinEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), cabinEqPage (p)
{
    setSize (1080, 720);
    addAndMakeVisible (cabinEqPage);
    addAndMakeVisible (visibilityButton);
    setResizable (true, false);
    
    visibilityButton.addListener (this);
}

CabinEqProcessorEditor::~CabinEqProcessorEditor()
{
    // The binaryClassificationPage will be automatically deleted as it is owned by the tabbedComponent
}

//==============================================================================
void CabinEqProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void CabinEqProcessorEditor::resized()
{
    cabinEqPage.setBounds (getLocalBounds());
    visibilityButton.setBounds (juce::Rectangle<int> (getWidth() - 100, getHeight() - 50, 100, 50));
}

void CabinEqProcessorEditor::buttonClicked (juce::Button* button)
{
    isVisible = ! isVisible;
    cabinEqPage.setVisible (isVisible);
    visibilityButton.setButtonText (isVisible ? "VISIBLE" : "INVISIBLE");
}
