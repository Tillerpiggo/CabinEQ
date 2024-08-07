/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId)
    : cabinEQGraph (p.getCurve (curveId)), processor (p), curveId (curveId)
{
    addAndMakeVisible (referenceSlider);
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    referenceSlider.addListener (this);
    
    updateEQNodes();
    
    addAndMakeVisible (cabinEQGraph);
    cabinEQGraph.addListener (this);
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    cabinEQGraph.removeListener();
}

void CabinEQPage::paint (juce::Graphics& g)
{
//    float currFreq = processor.getCurrPlayingFreq();
    g.fillAll (backgroundColor);
//    drawCurve (g, processor.getCurve (curveId), 4000);
//    drawDots (g);
}

void CabinEQPage::resized()
{
    int padding = 10;
    int sliderHeight = 50;
    
    // Calculate available height for the graph
    int graphHeight = getHeight() - sliderHeight - (3 * padding); // Extra padding for top and bottom
    
    // Set bounds for the cabinEQGraph with padding on all sides
    cabinEQGraph.setBounds(0, 0, getWidth(), graphHeight);
    
    // Set bounds for the referenceSlider with padding
    referenceSlider.setBounds(padding, getHeight() - sliderHeight - padding, getWidth() - (2 * padding), sliderHeight);
}

void CabinEQPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceSlider)
    {
        processor.setReferenceVolume (slider->getValue());
    }
}

void CabinEQPage::updateEQNodes()
{
    eqNodes = processor.getEQNodes(curveId);
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
