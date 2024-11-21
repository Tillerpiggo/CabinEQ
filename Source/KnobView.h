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
        virtual void addBands (std::vector<Band> bands) = 0;
    };
    
    KnobView();
    ~KnobView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    
private:
    void updateBands(); // update bands based on current factors
    void setIsOn (bool isOn);
    void generatePermutations();
    
    std::vector<Band> bands;
    std::vector<std::vector<Band>> permutations;
    std::vector<std::vector<Band>> finePermutations;
    
    Listener* listener;
    
    juce::Slider randomSlider;
    juce::Slider fineSlider;
    juce::Slider bandwidthSlider;
    juce::Slider spacingSlider;
    juce::Slider pitchSlider;
    juce::Slider gainSlider;
    juce::Label randomLabel;
    juce::Label fineLabel;
    juce::Label bandwidthLabel;
    juce::Label spacingLabel;
    juce::Label pitchLabel;
    juce::Label gainLabel;
    
    juce::TextButton addBandsButton { "Add Bands" };
    juce::TextButton onButton { "OFF" };
    
    bool isOn;
};
