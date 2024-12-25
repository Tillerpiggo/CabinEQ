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
class MultiBandStepBar  : public juce::Component
{
public:
    MultiBandStepBar();
    ~MultiBandStepBar() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setDataSource (CabinPeqGraphDataSource* dataSource);
    
private:
    CabinPeqGraphDataSource* dataSource = nullptr;
    std::vector<std::unique_ptr<MultiBandStepView>> stepViews;
};
