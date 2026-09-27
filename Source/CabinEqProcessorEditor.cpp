/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "CabinEqProcessorEditor.h"

CabinEqProcessorEditor::CabinEqProcessorEditor (CabinEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Pop-ups and dialogs that aren't inside the editor use the default look and feel
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
    setLookAndFeel (&lookAndFeel);

    // Read the saved size first: setting the limits resizes the editor, which would overwrite it
    const auto size = audioProcessor.getEditorSize();

    cabinEqPage = std::make_unique<CabinEqPage> (p);
    addAndMakeVisible (*cabinEqPage);

    setResizable (true, true);
    setResizeLimits (760, 460, 4000, 3000);
    setSize (size.x, size.y);
    isRememberingSize = true;
}

CabinEqProcessorEditor::~CabinEqProcessorEditor()
{
    cabinEqPage.reset();
    setLookAndFeel (nullptr);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
}

//==============================================================================
void CabinEqProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);
}

void CabinEqProcessorEditor::resized()
{
    if (cabinEqPage != nullptr)
    {
        cabinEqPage->setBounds (getLocalBounds());
        if (isRememberingSize)
            audioProcessor.setEditorSize ({ getWidth(), getHeight() });
    }
}
