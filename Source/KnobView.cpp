/*
  ==============================================================================

    KnobView.cpp
    Created: 20 Nov 2024 6:30:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "KnobView.h"

KnobView::KnobView()
{
    // Sliders
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.1f, 5.0f, 1.0f);
    addSliderAndLabel (&spacingSlider, &spacingLabel, "Spacing", 0.0f, 5.0f, 1.0f); // relative units
    addSliderAndLabel (&pitchSlider, &pitchLabel, "Pitch", -5.0f, 5.0f, 0.0f); // in octaves, from 800hz
    addSliderAndLabel (&gainSlider, &gainLabel, "Gain", 0.0f, 24.0f); // in db
    
    // Slider Actions
    addSliderAction (&bandwidthSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&spacingSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&pitchSlider, [this](juce::Slider*) {
        updateBands();
    });
    addSliderAction (&gainSlider, [this](juce::Slider*) {
        updateBands();
    });
    
    // Buttons
    addButton (&addBandsButton);
    addButton (&onButton);
    
    // Button Actions
    addButtonAction (&addBandsButton, [this](juce::Button*) {
        if (listener != nullptr)
        {
            listener->addBands (bands);
            setIsOn (false);
        }
    });
}

KnobView::~KnobView()
{
}

void KnobView::paint (juce::Graphics& g)
{
    g.fillAll (CONTROL_BAR_BACKGROUND_COLOR);
}

void KnobView::resized()
{
    Layout layout (getBounds(), 0.0f);
    layout.addRow ({ Space (80), Space (&bandwidthSlider) });
    layout.addRow ({ Space (80), Space (&spacingSlider) });
    layout.addRow ({ Space (80), Space (&pitchSlider) });
    layout.addRow ({ Space (80), Space (&gainSlider) });
    layout.addRow ({ Space (&addBandsButton), Space (&onButton, 80) });
}

void KnobView::setListener (Listener* listener)
{
    this->listener = listener;
}

void KnobView::updateBands()
{
    // Calculate out the values for bands
    
    // Start with the default bands
    float centerFreq = 800.0f;
    float spacingFactor = 0.5f; // octaves
    float spacingRatio = std::pow (2.0f, spacingFactor * spacingSlider.getValue());
    
    float ampl = gainSlider.getValue();
    float bandwidth = bandwidthSlider.getValue();
    Band::Type type = Band::Type::both;
    
    // Calculate bands & update listener
    std::vector<Band> provisionalBands = { 
        Band (0, centerFreq / spacingRatio, ampl, bandwidth, type),
        Band (0, centerFreq / spacingRatio, ampl, bandwidth, type),
        Band (0, centerFreq / spacingRatio, ampl, bandwidth, type)
    };
    
    
    if (listener != nullptr)
    {
        listener->setBands (provisionalBands);
        bands = provisionalBands;
    }
}
