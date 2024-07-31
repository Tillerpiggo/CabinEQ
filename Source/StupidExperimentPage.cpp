/*
  ==============================================================================

    StupidExperimentPage.cpp
    Created: 30 Jul 2024 6:21:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "StupidExperimentPage.h"

StupidExperimentPage::StupidExperimentPage (StartupMVPAudioProcessor& p)
    : processor (p)
{
    addAndMakeVisible (slider);
    addAndMakeVisible (startButton);
    
    slider.addListener (this);
    startButton.addListener (this);
    
    slider.setRange (-36.0f, 36.0f);
}

StupidExperimentPage::~StupidExperimentPage()
{
    slider.removeListener (this);
    startButton.removeListener (this);
}

void StupidExperimentPage::paint (juce::Graphics&)
{
    
}

void StupidExperimentPage::resized()
{
    auto area = getLocalBounds();
    auto sliderArea = area.removeFromTop(area.getHeight() / 2);
    slider.setBounds(sliderArea.reduced(10));
    
    auto buttonArea = area.removeFromTop(area.getHeight() / 2);
    startButton.setBounds(buttonArea.reduced(10));
}

void StupidExperimentPage::sliderValueChanged (juce::Slider *slider)
{
    // I don't think we actually need to do anything here
}

void StupidExperimentPage::buttonClicked (juce::Button *button)
{
    if (button == &startButton)
    {
        currFreq = MIN_FREQ;
        startTimer (40);
//        processor.startSineSweep (currFreq, slider.getValue());
        processor.startCalibratingEQNode(EQNode (-1, currFreq, slider.getValue(), 0.0f));
        processor.clearEQNodes();
    }
}

void StupidExperimentPage::timerCallback()
{
    step++;
    
    currFreq *= 1.001;
    std::cout << "currFreq" << std::endl;
//    processor.updateSineSweep (currFreq, slider.getValue());
    processor.updateCalibratingEQNode (EQNode (-1, currFreq, slider.getValue(), 0.0f));
    if (step > 50)
    {
        processor.addEQNode (currFreq, slider.getValue(), 0.0f);
        step = 0;
    }
    
    if (currFreq > MAX_FREQ)
    {
        stopTimer();
        processor.endTesting();
        processor.endSineSweep();
    }
}
