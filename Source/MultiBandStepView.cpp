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
    drawBounds (g);
    drawCurve (g);
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

void MultiBandStepView::drawBounds (juce::Graphics& g)
{
    // Draw the background
    g.setColour (BACKGROUND_COLOUR);
    g.fillRect (getBounds());
    
    // Draw the border
    g.setColour (BORDER_COLOUR);
    g.drawRect (getBounds());
}

void MultiBandStepView::drawCurve (juce::Graphics& g)
{
    // Draw the curve according to the multi step bands
    g.setColour (CURVE_COLOUR);
    juce::Path path;
    
    // Draw curve with NUM_POINTS points
    for (int i = 0; i < NUM_POINTS; ++i)
    {
        float t = static_cast<float> (i) / static_cast<float> (NUM_POINTS);
        
//        float freq = frequencyAtTime (t);
//        float ampl = 
        
        // Make MultiBandStep give a bandEQCurve
    }
}
