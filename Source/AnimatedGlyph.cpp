/*
  ==============================================================================

    AnimatedGlyph.cpp
    Created: 14 Nov 2024 4:21:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AnimatedGlyph.h"

AnimatedGlyph::AnimatedGlyph()
{}

void AnimatedGlyph::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
}

void AnimatedGlyph::setStrokeWidth (float strokeWidth)
{
    this->strokeWidth = strokeWidth;
}

void AnimatedGlyph::paint (juce::Graphics& g)
{
    // TODO: draw the glyph
    drawStrokes (g);
    drawDot (g);
}

void AnimatedGlyph::resized()
{
    // Do nothing for now...
}

void AnimatedGlyph::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
}

void AnimatedGlyph::timerCallback()
{
    repaint();
}

void AnimatedGlyph::drawStrokes (juce::Graphics& g)
{
    if (! glyph.has_value())
        return;
    
    auto strokes = glyph->getStrokes();
    juce::Path path;
    for (const auto& stroke : strokes)
    {
        auto [startPoint, endPoint] = stroke.getEndPoints();
        
        startPoint = getPointInBounds (startPoint);
        endPoint = getPointInBounds (endPoint);
        
        path.addLineSegment(juce::Line<float> (startPoint, endPoint), strokeWidth);
    }
    
    g.setColour (juce::Colours::lightblue);
    g.fillPath (path);
}

void AnimatedGlyph::drawDot (juce::Graphics& g)
{
    if (! glyph.has_value() || dataSource == nullptr)
        return;
    
    auto point = glyph->positionAtTime (dataSource->getCurrTime());
    point = getPointInBounds (point);
    g.setColour (juce::Colours::darkblue);
    g.fillEllipse (point.x, point.y, strokeWidth, strokeWidth);
}

juce::Point<float> AnimatedGlyph::getPointInBounds (juce::Point<float> point)
{
    float xScaled = (point.x + 1.0f) / 2.0f;
    float yScaled = (point.y + 1.0f) / 2.0f;
    
    float xInBounds = getX() + xScaled * getWidth();
    float yInBounds = getY() + yScaled * getHeight();
    
    return { xInBounds, yInBounds };
}
