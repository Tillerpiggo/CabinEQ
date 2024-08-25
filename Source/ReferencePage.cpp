/*
  ==============================================================================

    ReferencePage.cpp
    Created: 24 Aug 2024 10:26:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ReferencePage.h"

ReferencePage::ReferencePage(StartupMVPAudioProcessor& p)
    : processor (p)
{
    addAndMakeVisible (referenceVolume1);
    addAndMakeVisible (referenceVolume2);
    
    referenceVolume1.addListener (this);
    referenceVolume2.addListener (this);
    
    referenceVolume1.setRange (-12.0f, 12.0f);
    referenceVolume2.setRange (-12.0f, 12.0f);
    referenceVolume1.setSliderStyle (juce::Slider::LinearHorizontal);
    referenceVolume2.setSliderStyle (juce::Slider::LinearHorizontal);
}

ReferencePage::~ReferencePage()
{
    referenceVolume1.removeListener (this);
    referenceVolume2.removeListener (this);
}

void ReferencePage::resized()
{
    auto bounds = getLocalBounds();
    referenceVolume1.setBounds (bounds.removeFromTop (100));
    referenceVolume2.setBounds (bounds.removeFromTop (100));
}

void ReferencePage::paint(juce::Graphics& g)
{
    
}

void ReferencePage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceVolume1)
    {
        processor.setReferenceVolume1 (referenceVolume1.getValue());
    }
    else if (slider == &referenceVolume2)
    {
        processor.setReferenceVolume2 (referenceVolume2.getValue());
    }
    
    processor.updatePlayingReferenceFreqs();
}

void ReferencePage::sliderDragStarted (juce::Slider *slider)
{
    processor.startPlayingReferenceFreqs();
}

void ReferencePage::sliderDragEnded (juce::Slider *slider)
{
    processor.updatePlayingReferenceFreqs();
}
