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
    drawPlayingDot (g);
    drawCenterDot (g);
}

void AnimatedGlyph::resized()
{
    // Do nothing for now...
}

void AnimatedGlyph::mouseMove (const juce::MouseEvent &event)
{
    updateHoveringStatus (event);
}

void AnimatedGlyph::mouseDown (const juce::MouseEvent &event)
{
    
}

void AnimatedGlyph::mouseDrag (const juce::MouseEvent &event)
{
    auto pos = normalizedPositionForMouseEvent (event);
    
    // If we're hovering over the control dot, drag the position
    if (isHovering && listener != nullptr && dataSource != nullptr)
    {
        listener->setCenterPos (pos);
        centerPos = dataSource->getCenterPos();
    }
}

void AnimatedGlyph::mouseUp (const juce::MouseEvent &event)
{
    
}

void AnimatedGlyph::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    // use deltaY to increase/decrease size factor, with bounds
    float newSizeFactor = sizeFactor;
    newSizeFactor += wheel.deltaY;
    newSizeFactor = std::min (std::max (newSizeFactor, 0.1f), 1.0f);
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->setSizeFactor (newSizeFactor);
        sizeFactor = dataSource->getSizeFactor();
        centerPos = dataSource->getCenterPos();
    }
        
}

void AnimatedGlyph::setListener (Listener* listener)
{
    this->listener = listener;
}

void AnimatedGlyph::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
    this->centerPos = dataSource->getCenterPos();
    this->sizeFactor = dataSource->getSizeFactor();
}

void AnimatedGlyph::timerCallback()
{
    repaint();
}

void AnimatedGlyph::updateHoveringStatus (const juce::MouseEvent& event)
{
    auto pos = normalizedPositionForMouseEvent (event);
    
    // Figure out if we're hovering over the center node
    isHovering = centerPos.getDistanceFrom (pos) < 0.2;
}

juce::Point<float> AnimatedGlyph::normalizedPositionForMouseEvent (const juce::MouseEvent& event)
{
    float x = event.getPosition().x;
    float y = event.getPosition().y;
    return getNormalizedPositionForPoint ({ x, y });
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

void AnimatedGlyph::drawPlayingDot (juce::Graphics& g)
{
    if (! glyph.has_value() || dataSource == nullptr)
        return;
    
    auto [point, _] = glyph->positionAtTime (dataSource->getCurrTime());
    point = getPointInBounds (point);
    
    float dotRadius = strokeWidth * 2.0f * strokeWidthFactor;
    drawDot (g, point, dotRadius, juce::Colours::darkblue, false);
    
}

void AnimatedGlyph::drawCenterDot (juce::Graphics& g)
{
    juce::Point<float> centerPoint = getPointInBounds ({ 0.0f, 0.0f });
    juce::Colour dotColour = juce::Colours::lightgreen;
    float dotRadius = 8.0f;
    drawDot (g, centerPoint, dotRadius, dotColour, isHovering);
}

void AnimatedGlyph::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour, bool isSelected)
{
    // Draw dot outside
    g.setColour (dotColour.withAlpha (isSelected ? 1.0f : 0.5f));
    g.fillEllipse (point.x - dotRadius - DOT_PADDING, point.y - dotRadius - DOT_PADDING, (dotRadius + DOT_PADDING) * 2, (dotRadius + DOT_PADDING) * 2);
    
    // Draw dot center
    g.setColour (dotColour.withAlpha (1.0f));
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

juce::Point<float> AnimatedGlyph::getPointInBounds (juce::Point<float> point)
{
    float padding = 0.0f;
    
    // Scale according to sizeFactor and centerPos
    point.x *= sizeFactor;
    point.y *= sizeFactor;
    point.x += centerPos.x;
    point.y += centerPos.y;
    
    float xScaled = (point.x + 1.0f) / 2.0f;
    float yScaled = (-point.y + 1.0f) / 2.0f;
    
    
    
    float xInBounds = padding + xScaled * (getWidth() - padding * 2.0f);
    float yInBounds = padding + yScaled * (getHeight() - padding * 2.0f);
    
    return { xInBounds, yInBounds };
}

juce::Point<float> AnimatedGlyph::getNormalizedPositionForPoint (juce::Point<float> point)
{
    float x = point.x;
    float y = point.y;
    
    float normalizedX = (2.0f * x / getWidth()) - 1.0f;
    float normalizedY = (2.0f * y / getHeight()) - 1.0f;
    normalizedY *= -1;
    
    return { normalizedX, normalizedY };
}
