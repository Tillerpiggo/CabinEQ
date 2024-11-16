/*
  ==============================================================================

    AnimatedGlyph.h
    Created: 14 Nov 2024 4:21:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"

// This class takes a data source and constantly animates a glyph reactively
class AnimatedGlyph  : public juce::Component,
                       public juce::Timer
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void setSizeFactor (float sizeFactor) = 0;
        virtual void setCenterPos (juce::Point<float> centerPos) = 0;
    };
    
    class DataSource
    {
    public:
        virtual ~DataSource() = default;
        virtual float getSizeFactor() = 0;
        virtual juce::Point<float> getCenterPos() = 0;
        virtual float getCurrTime() = 0;
    };
    
    AnimatedGlyph();
    ~AnimatedGlyph() override;
    
    void setGlyph (Glyph glyph); // uses glyph + time to figure out details of what to display
    void setStrokeWidthFactor (float strokeWidthFactor);
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    void setDataSource (DataSource* dataSource);
    
    void timerCallback() override;
    
private:
    void drawStrokes (juce::Graphics& g);
    void drawDot (juce::Graphics& g);
    juce::Point<float> getPointInBounds (juce::Point<float> point);
    
    Listener* listener = nullptr;
    DataSource* dataSource = nullptr;
    
    std::optional<Glyph> glyph;
    float strokeWidth = 3.0f;
    float strokeWidthFactor = 1.0f;
};
