/*
  ==============================================================================

    2DSlider.cpp
    Created: 27 Dec 2024 12:10:19am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "DimensionalSlider.h"

DimensionalSlider::DimensionalSlider()
{
    
}

DimensionalSlider::~DimensionalSlider()
{
    
}

void DimensionalSlider::paint (juce::Graphics& g)
{
    drawBounds (g);
    drawDot (g);
}

void DimensionalSlider::resized()
{
    
}

void DimensionalSlider::mouseMove (const juce::MouseEvent &event)
{
//    updatePosFromEvent (event);
}

void DimensionalSlider::mouseDown (const juce::MouseEvent &event)
{
    updatePosFromEvent (event);
}

void DimensionalSlider::mouseDrag (const juce::MouseEvent &event)
{
    updatePosFromEvent (event);
}

void DimensionalSlider::mouseUp (const juce::MouseEvent &event)
{
    updatePosFromEvent (event);
}

void DimensionalSlider::setListener (Listener* listener)
{
    this->listener = listener;
}

void DimensionalSlider::setPosition (juce::Point<float> pos)
{
    this->pos = pos;
    repaint();
}

void DimensionalSlider::drawBounds (juce::Graphics& g)
{
    g.setColour (BACKGROUND_COLOUR);
    g.fillRect (getBounds());
    g.setColour (BORDER_COLOUR);
    g.drawRect (getBounds());
}

void DimensionalSlider::drawDot (juce::Graphics& g)
{
    auto coords = coordsFromPos (pos);
    g.setColour (DOT_COLOUR);
    g.fillEllipse (coords.x - DOT_RADIUS, coords.y - DOT_RADIUS, DOT_RADIUS * 2.0f, DOT_RADIUS * 2.0f);
}

void DimensionalSlider::updatePosFromEvent (const juce::MouseEvent& event)
{
    auto pos = posFromCoords (event.getPosition().toFloat());
    this->pos = pos;
    if (listener != nullptr)
        listener->positionChanged (pos);
    repaint();
}

juce::Point<float> DimensionalSlider::coordsFromPos (juce::Point<float> normalizedPos)
{
    float positiveX = (normalizedPos.x + 1.0f) / 2.0f;
    float positiveY = (-normalizedPos.y + 1.0f) / 2.0f;
    
    float coordsX = (float) getWidth() * positiveX;
    float coordsY = (float) getHeight() * positiveY;
    
    return { coordsX, coordsY };
}

juce::Point<float> DimensionalSlider::posFromCoords (juce::Point<float> coords)
{
    float positiveX = coords.x / (float) getWidth();
    float positiveY = coords.y / (float) getHeight();
    
    positiveX = std::min (std::max (positiveX, 0.0f), 1.0f);
    positiveY = std::min (std::max (positiveY, 0.0f), 1.0f);
    
    float normalizedX = positiveX * 2.0f - 1.0f;
    float normalizedY = positiveY * 2.0f - 1.0f;
    
    return { normalizedX, -normalizedY };
}
