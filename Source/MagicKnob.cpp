/*
  ==============================================================================

    MagicKnob.cpp
    Created: 30 Dec 2024 6:47:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MagicKnob.h"

MagicKnob::MagicKnob()
    : magicDecoder (make_unique<SimpleDecoder>())
{
    // Add sliders
    addSliderAndLabel (&slider1, &label1, "Roll", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider2, &label2, "Pitch", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider3, &label3, "Yaw", 0.0f, 1.0f, 0.5f);
    
    // Add slider actions
    addSliderAction (&slider1, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider2, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider3, [this](juce::Slider*) {
        updateListener();
    });
}

MagicKnob::~MagicKnob()
{
    
}

void MagicKnob::paint (juce::Graphics& g)
{
    // Do nothing
}

void MagicKnob::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 4.0f);
    layout.addRow ({ Space (&slider1), Space (&slider2), Space (&slider3) });
    layout.updateComponentBounds();
}

void MagicKnob::setListener (Listener* listener)
{
    this->listener = listener;
}

void MagicKnob::updateListener()
{
    if (listener == nullptr)
        return;
    
    // Compute the bands
    auto bands = magicDecoder->decode (slider1.getValue(), slider2.getValue(), slider3.getValue());
    listener->setBands (bands);
}
