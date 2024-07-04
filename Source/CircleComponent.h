/*
  ==============================================================================

    CircleComponent.h
    Created: 3 Jul 2024 4:54:25pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CircleComponent : public juce::Component
{
public:
    CircleComponent() {}

    void setColor(juce::Colour color)
    {
        circleColor = color;
        repaint();  // Repaint the component to reflect the new color
    }
    
    void paint(juce::Graphics& g) override
    {
        // Set the color for the circle
        g.setColour(circleColor);

        // Get the bounds of the component
        auto bounds = getLocalBounds().toFloat();

        // Draw a filled ellipse (circle)
        g.fillEllipse(bounds);
    }

private:
    juce::Colour circleColor { juce::Colours::transparentBlack };  // Default to transparent color
};
