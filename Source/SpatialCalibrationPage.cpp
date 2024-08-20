/*
  ==============================================================================

    SpatialCalibrationPage.cpp
    Created: 18 Aug 2024 9:49:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#include "SpatialCalibrationPage.h"

SpatialCalibrationPage::SpatialCalibrationPage (StartupMVPAudioProcessor& p)
    : processor (p)
{
    volumeSlider.setRange (-24.0f, 24.0f);
    panSlider.setRange (-12.0f, 12.0f);
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    panSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (panSlider);
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);
    
    volumeSlider.addListener (this);
    panSlider.addListener (this);
    prevButton.addListener (this);
    nextButton.addListener (this);
    processor.addListener (this);
    
    // Initialize freqs to have 30 evenly spaced frequencies across the spectrum (20hz - 15000hz)
    float startFreq = 20.0f;
    float endFreq = 15000.0f;
    int numFreqs = 30;
    
    float ratio = std::pow (endFreq / startFreq, 1.0f / (numFreqs - 1));

    for (int i = 0; i < numFreqs; ++i)
    {
        freqs.push_back (startFreq * std::pow(ratio, i));
    }
    
    prevButton.setEnabled (false);
    
    didLoadData();
}

SpatialCalibrationPage::~SpatialCalibrationPage()
{
    volumeSlider.removeListener (this);
    panSlider.removeListener (this);
    prevButton.removeListener (this);
    nextButton.removeListener (this);
}

void SpatialCalibrationPage::paint (juce::Graphics&)
{
    
}

void SpatialCalibrationPage::resized()
{
    auto area = getLocalBounds();
    
    // Divide the area into three vertical sections
    auto sliderArea = area.removeFromTop(area.getHeight() / 2);
    auto buttonArea = area;
    
    // Divide the slider area for the two sliders
    volumeSlider.setBounds(sliderArea.removeFromTop(sliderArea.getHeight() / 2));
    panSlider.setBounds(sliderArea);
    
    // Divide the button area for the two buttons
    prevButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2));
    nextButton.setBounds(buttonArea);
}

void SpatialCalibrationPage::sliderDragStarted (juce::Slider *slider)
{
    float ampl = volumeSlider.getValue();
    float pan = panSlider.getValue();
    processor.startCalibratingEQNode (EQNode (currNodeId, freqs[currNodeId], ampl, pan));
}

void SpatialCalibrationPage::sliderDragEnded (juce::Slider *slider)
{
    processor.endCalibratingEQNode();
    processor.updateEQNode (currNodeId, freqs[currNodeId], volumeSlider.getValue(), panSlider.getValue(), profileId);
}

void SpatialCalibrationPage::sliderValueChanged (juce::Slider *slider)
{
    float ampl = volumeSlider.getValue();
    float pan = panSlider.getValue();
    processor.updateCalibratingEQNode (EQNode (currNodeId, freqs[currNodeId], ampl, pan));
    processor.updateEQNode (currNodeId, freqs[currNodeId], volumeSlider.getValue(), panSlider.getValue(), profileId);
}

void SpatialCalibrationPage::buttonClicked (juce::Button *button)
{
    if (button == &prevButton)
    {
        currNodeId--;
    }
    else if (button == &nextButton)
    {
        currNodeId++;
    }
    
    auto eqNode = processor.getEQNodeWithId (profileId, currNodeId);
    volumeSlider.setValue (eqNode->amplitude);
    panSlider.setValue (eqNode->pan);
    
    prevButton.setEnabled (currNodeId > 0);
    nextButton.setEnabled (currNodeId < freqs.size() - 1);
}

void SpatialCalibrationPage::didLoadData()
{
    auto eqNodes = processor.getEQNodes (profileId);
    std::cout << "didLoadData()" << std::endl;
    if (eqNodes.size() == 0)
    {
        std::cout << "initializing data" << std::endl;
        processor.addProfile (profileId);
        for (const auto freq : freqs)
        {
            processor.addEQNode (freq, 0.0f, 0.0f, profileId);
        }
    }
}

*/
