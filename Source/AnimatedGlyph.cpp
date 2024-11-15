/*
  ==============================================================================

    AnimatedGlyph.cpp
    Created: 14 Nov 2024 4:21:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AnimatedGlyph.h"

AnimatedGlyph::AnimatedGlyph()
{
    startTimer (5);
}

AnimatedGlyph::~AnimatedGlyph()
{
    stopTimer();
}

void AnimatedGlyph::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
}

void AnimatedGlyph::setStrokeWidthFactor (float strokeWidthFactor)
{
    this->strokeWidthFactor = strokeWidthFactor;
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
        for (int i = 0; i < stroke.getPoints().size() - 1; ++i)
        {
            auto startPoint = stroke.getPoints()[i];
            auto endPoint = stroke.getPoints()[i + 1];
            
            startPoint = getPointInBounds (startPoint);
            endPoint = getPointInBounds (endPoint);
            
            path.addLineSegment(juce::Line<float> (startPoint, endPoint), strokeWidth * strokeWidthFactor);
        }
    }
    
    g.setColour (juce::Colours::lightblue);
    g.fillPath (path);
}

void AnimatedGlyph::drawDot (juce::Graphics& g)
{
    if (! glyph.has_value() || dataSource == nullptr)
        return;
    
    auto [point, _] = glyph->positionAtTime (dataSource->getCurrTime());
    point = getPointInBounds (point);
    
    g.setColour (juce::Colours::darkblue);
    float dotRadius = strokeWidth * 2.0f * strokeWidthFactor;
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
}

juce::Point<float> AnimatedGlyph::getPointInBounds (juce::Point<float> point)
{
    float padding = 20.0f;
    
    float xScaled = (point.x + 1.0f) / 2.0f;
    float yScaled = (-point.y + 1.0f) / 2.0f;
    
    float xInBounds = padding + getX() + xScaled * (getWidth() - padding * 2.0f);
    float yInBounds = padding + getY() + yScaled * (getHeight() - padding * 2.0f);
    
    return { xInBounds, yInBounds };
}
