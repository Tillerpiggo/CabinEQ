/*
  ==============================================================================

    MultiBandStepView.h
    Created: 25 Dec 2024 1:22:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"

// This draws a single multi step representation that can be selected, enabled/disabled, hovered over, etc.
class MultiBandStepView  : public juce::Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void onStepEnabled (int stepId, bool isEnabled) = 0;
        virtual void onStepSelected (int id) = 0;
    };
    
    MultiBandStepView();
    MultiBandStepView (MultiBandStep step);
    ~MultiBandStepView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    void setMultiBandStep (MultiBandStep step);
    
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    
private:
    void drawBounds (juce::Graphics& g);
    void drawCurve (juce::Graphics& g);
    
    Listener* listener = nullptr;
    std::optional<MultiBandStep> step;
    
    
    // Visual variables
    bool isHovering = false;
    
    // Visual constants
    juce::Colour BACKGROUND_COLOUR = juce::Colours::teal;
    juce::Colour BORDER_COLOUR = juce::Colours::cyan;
    juce::Colour CURVE_COLOUR = juce::Colours::green;
    int NUM_POINTS = 50; // we shouldn't need as many for the little preview
};
