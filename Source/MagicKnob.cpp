/*
  ==============================================================================

    MagicKnob.cpp
    Created: 30 Dec 2024 6:47:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MagicKnob.h"

MagicKnob::MagicKnob()
    : magicDecoder (std::make_unique<TensorflowDecoder>())
{
    // Add sliders
    addSliderAndLabel (&slider1, &label1, "1", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider2, &label2, "2", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider3, &label3, "3", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider4, &label4, "4", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider5, &label5, "5", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider6, &label6, "6", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider7, &label7, "7", 0.0f, 1.0f, 0.5f);
    addSliderAndLabel (&slider8, &label8, "8", 0.0f, 1.0f, 0.5f);
    
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
    addSliderAction (&slider4, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider5, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider6, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider7, [this](juce::Slider*) {
        updateListener();
    });
    addSliderAction (&slider8, [this](juce::Slider*) {
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
    layout.addRow ({ Space (&slider1), Space (&slider2), Space (&slider3), Space (&slider4) });
    layout.addRow ({ Space (&slider5), Space (&slider6), Space (&slider7), Space (&slider8) });
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
    auto bands = magicDecoder->decode ({ (float) slider1.getValue(), (float) slider2.getValue(), (float) slider3.getValue(), (float) slider4.getValue(), (float) slider5.getValue(), (float) slider6.getValue(), (float) slider7.getValue(), (float) slider8.getValue() });
    if (bands.size() > 0)
        listener->setBands (bands);
}
