/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId)
    : processor (p), cabinEQGraph()
{
    dropdownProfiles.addItem ("+ Add Profile", 1);
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    
    cabinEQGraph.addListener (this);
    dropdownProfiles.addListener (this);
    referenceSlider.addListener (this);
    
    addAndMakeVisible (cabinEQGraph);
    addAndMakeVisible (dropdownProfiles);
    addAndMakeVisible (referenceSlider);
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    cabinEQGraph.removeListener();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColor);
}

void CabinEQPage::resized()
{
    int padding = 10;
    int graphHeight = getHeight() - (2 * padding) - 30; // Adjust for dropdown height

    cabinEQGraph.setBounds(0, 0, getWidth(), graphHeight);
    dropdownProfiles.setBounds(padding, getHeight() - padding - 30, getWidth() - (2 * padding), 30);
}

// ====================================================
int CabinEQPage::addNode (float freq, float ampl)
{
    return processor.addEQNode (freq, ampl, 0.0f, curveId);
}

void CabinEQPage::updateNode (int id, float freq, float ampl)
{
    processor.updateEQNode (id, freq, ampl, 0.0f, curveId);
}

void CabinEQPage::removeNode (int id)
{
    processor.removeEQNode (id, curveId);
}

void CabinEQPage::startPlayingValueAt (float freq, float ampl)
{
    processor.startCalibratingEQNode (EQNode (-1, freq, ampl, 0.0f));
}

void CabinEQPage::playValueAt (float freq, float ampl)
{
    processor.updateCalibratingEQNode (EQNode (-1, freq, ampl, 0.0f));
}

void CabinEQPage::testValueAt (float freq)
{
    processor.startTestingAt (freq, curveId);
}

void CabinEQPage::stopPlaying()
{
    processor.endCalibratingEQNode();
}

void CabinEQPage::stopTesting()
{
    processor.endTesting();
}

float CabinEQPage::getCurrPlayingFreq()
{
    return processor.getCurrPlayingFreq();
}

float CabinEQPage::getCurrTestingFreq()
{
    return processor.getCurrTestingFreq();
}

// ====================================================
void CabinEQPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceSlider)
    {
        processor.setReferenceVolume (slider->getValue());
    }
}

void CabinEQPage::comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &dropdownProfiles)
    {
        // Do some action because something was selected...
    }
}
