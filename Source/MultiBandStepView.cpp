/*
  ==============================================================================

    MultiBandStepView.cpp
    Created: 25 Dec 2024 1:22:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MultiBandStepView.h"

MultiBandStepView::MultiBandStepView()
    : step (std::nullopt)
{
    
}

MultiBandStepView::MultiBandStepView (MultiBandStep step)
    : step (step)
{
    
}

MultiBandStepView::~MultiBandStepView()
{
    
}

void MultiBandStepView::paint (juce::Graphics& g)
{
    
}

void MultiBandStepView::resized()
{
    
}

void MultiBandStepView::setListener (Listener* listener)
{
    this->listener = listener;
}

void MultiBandStepView::setMultiBandStep (MultiBandStep step)
{
    this->step = step;
}

void MultiBandStepView::mouseEnter (const juce::MouseEvent& event)
{
    isHovering = true;
}

void MultiBandStepView::mouseExit (const juce::MouseEvent& event)
{
    isHovering = false;
}
