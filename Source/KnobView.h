/*
  ==============================================================================

    KnobView.h
    Created: 20 Nov 2024 6:30:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "UIConstants.h"
#include "BandProfile.h"
#include "Layout.h"

// This provides a UI to add multiple bands at once, as well as preview their effects on the sound
class KnobView  : public BuildableComponent
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void setBands (std::vector<Band> provisionalBands) = 0;
        virtual void setIsOn (bool isOn) = 0;
        virtual int addBands (std::vector<Band> bands) = 0;
    };
    
    KnobView();
    ~KnobView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    
private:
    void updateBands(); // update bands based on current factors
    void setIsOn (bool isOn);
    
    std::vector<Band> bands;
    
    Listener* listener;
    
    juce::Slider bandwidthSlider;
    juce::Slider spacingSlider;
    juce::Slider pitchSlider;
    juce::Slider gainSlider;
    juce::Label bandwidthLabel;
    juce::Label spacingLabel;
    juce::Label pitchLabel;
    juce::Label gainLabel;
    
    juce::TextButton addBandsButton { "Add Bands" };
    juce::TextButton onButton { "ON" };
    
    bool isOn;
};
