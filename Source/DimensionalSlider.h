/*
  ==============================================================================

    2DSlider.h
    Created: 27 Dec 2024 12:10:19am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DimensionalSlider  : juce::Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void positionChanged (juce::Point<float> pos);
    };
    
    DimensionalSlider();
    ~DimensionalSlider() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    
    void setListener (Listener* listener);
    void setPosition (juce::Point<float> pos);
    
    
private:
    void drawBounds (juce::Graphics& g);
    void drawDot (juce::Graphics& g);
    
    void updatePosFromEvent (const juce::MouseEvent& event);
    juce::Point<float> coordsFromPos (juce::Point<float> normalizedPos);
    juce::Point<float> posFromCoords (juce::Point<float> coords);
    
    Listener* listener;
    juce::Point<float> pos { 0.0f, 0.0f };
    
    // Visual constants
    juce::Colour BACKGROUND_COLOUR = juce::Colours::black;
    juce::Colour BORDER_COLOUR = juce::Colours::white;
    juce::Colour DOT_COLOUR = juce::Colours::yellow;
    float DOT_RADIUS = 4.0f;
    
    
};
