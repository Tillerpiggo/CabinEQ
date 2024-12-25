/*
  ==============================================================================

    MultiBandStepBar.h
    Created: 25 Dec 2024 1:18:56pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Listeners.h"
#include "Layout.h"
#include "MultiBandStepView.h"

// This displays a scrollable list of multi band steps that allows you to select and add multi band steps
class MultiBandStepBar  : public juce::Component,
                          public MultiBandStepView::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void setStepSelected (int id) = 0;
    };
    
    MultiBandStepBar();
    ~MultiBandStepBar() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    void setBackendListener (CabinPeqGraphListener* backendListener);
    void updateBandProfile (BandProfile bandProfile);
    
    // MultiBandStepView::Listener methods
    void onStepEnabled (int stepId, bool isEnabled) override;
    void onStepSelected (int stepId) override;
    
private:
    BandProfile bandProfile;
    
    Listener* listener = nullptr;
    CabinPeqGraphListener* backendListener = nullptr;
    std::vector<std::unique_ptr<MultiBandStepView>> stepViews;
};
