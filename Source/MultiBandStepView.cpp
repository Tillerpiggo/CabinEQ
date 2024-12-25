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
    juce::Path path;
    
    if (! step.has_value())
    {
        path.startNewSubPath (0, getHeight() / 2.0f);
        path.lineTo (getWidth(), getHeight() / 2.0f);
    }
    else
    {
        // Draw curve with NUM_POINTS points
        for (int i = 0; i < NUM_POINTS; ++i)
        {
            float t = static_cast<float> (i) / static_cast<float> (NUM_POINTS);
            
            float freq = frequencyAtTime (t);
            float ampl = step->dbAtFrequency (freq);
            juce::Point<float> coords = coordsForTimeAndAmplitude (t, ampl);
            if (i == 0)
                path.startNewSubPath (coords);
            else
                path.lineTo (coords);
        }
    }
    g.setColour (CURVE_COLOUR);
    g.strokePath (path, juce::PathStrokeType (CURVE_THICKNESS));
}

float MultiBandStepView::frequencyAtTime (float t) const
{
    // Scale logarithmically based on the visible window
    float logMinFreqShowing = std::log (MIN_FREQ);
    float logMaxFreqShowing = std::log (MAX_FREQ);
    float freq = std::exp (logMinFreqShowing + t * (logMaxFreqShowing - logMinFreqShowing));
    return freq;
}

juce::Point<float> MultiBandStepView::coordsForTimeAndAmplitude (float t, float ampl) const
{
    // Calculate (x, y) coords and return
    float x = getWidth() * t;
    float y = getHeight() * (1.0f - (ampl - MIN_DB) / (MAX_DB - MIN_DB));
    return { x, y };
}
