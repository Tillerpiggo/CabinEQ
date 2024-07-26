/*
  ==============================================================================

    EQNodeController.cpp
    Created: 25 Jul 2024 9:00:37pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "EQNodeController.h"

EQNodeController::EQNodeController (int id, float frequency, float amplitude, float pan)
    : id (id), frequency (frequency), amplitude (amplitude), pan (pan)
{
    frequencySlider.setRange (20.0f, 20000.0f, 0.5);
    frequencySlider.setSkewFactorFromMidPoint (1000.0f);
    frequencySlider.setTextValueSuffix (" hz");
    frequencySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    frequencySlider.setValue (frequency);
    
    amplitudeSlider.setRange (-48.0f, 48.0f, 0.05);
    amplitudeSlider.setTextValueSuffix (" dB");
    amplitudeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    amplitudeSlider.setValue (amplitude);
    
    panSlider.setRange (-24.0f, 24.0f, 0.01);
    panSlider.setTextValueSuffix (" dB");
    panSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    panSlider.setValue (amplitude);
    
    frequencySlider.addListener (this);
    amplitudeSlider.addListener (this);
    panSlider.addListener (this);
    removeButton.addListener (this);

    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(amplitudeSlider);
    addAndMakeVisible(panSlider);
    addAndMakeVisible(removeButton);
}

EQNodeController::EQNodeController (EQNode eqNode)
    : EQNodeController (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan)
{}

EQNodeController::~EQNodeController()
{
    frequencySlider.removeListener (this);
    amplitudeSlider.removeListener (this);
    panSlider.removeListener (this);
    removeButton.removeListener (this);
}

void EQNodeController::resized()
{
    auto area = getLocalBounds();
    auto padding = 10;
    auto sliderHeight = (area.getHeight() * 3 / 4) / 3;
    
    frequencySlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
    amplitudeSlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
    panSlider.setBounds(area.removeFromTop(sliderHeight).reduced(padding));
    removeButton.setBounds(area.reduced(padding));
}

void EQNodeController::paint (juce::Graphics& g)
{
    // TODO: Paint colors here
}


void EQNodeController::sliderValueChanged (juce::Slider *slider)
{
    // Do nothing, for now
}

void EQNodeController::sliderDragStarted (juce::Slider *slider)
{
    if (slider == &frequencySlider || slider == &amplitudeSlider || slider == &panSlider)
    {
        listener->eqNodeChanged (id, frequencySlider.getValue(), amplitudeSlider.getValue(), panSlider.getValue());
    }
}

void EQNodeController::sliderDragEnded (juce::Slider *slider)
{
    // Do nothing, for now
}

void EQNodeController::buttonClicked (juce::Button *button)
{
    if (button == &removeButton)
    {
        listener->removeButtonClicked (id);
    }
}

void EQNodeController::setListener (EQNodeControllerListener *listener)
{
    this->listener = listener;
}

void EQNodeController::removeListener()
{
    this->listener = nullptr;
}
