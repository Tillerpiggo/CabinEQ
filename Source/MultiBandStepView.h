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
        virtual ~Listener() = default;
        
        virtual void setMultiBandStepEnabledTo (bool isEnabled, int id) = 0;
        virtual void onMultiBandStepSelected (int id) = 0;
    };
    
    MultiBandStepView (MultiBandStep multiBandStep);
    ~MultiBandStepView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setMultiBandStep (MultiBandStep step);
    
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    
private:
    MultiBandStep multiBandStep;
    
    // Visual constants
};
