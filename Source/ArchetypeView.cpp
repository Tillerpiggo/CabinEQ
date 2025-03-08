/*
  ==============================================================================

    ArchetypeView.cpp
    Created: 22 Dec 2024 9:07:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArchetypeView.h"

ArchetypeView::ArchetypeView (ArchetypalGlyph archetype)
    : archetype (archetype)
{
    
}

ArchetypeView::~ArchetypeView()
{
    
}

void ArchetypeView::paint (juce::Graphics& g)
{
    juce::Path path;
    std::vector<Stroke> strokes = archetype.getStrokes();
    for (const auto& stroke : strokes)
    {
        auto points = stroke.getPoints();
        for (int i = 0; i < points.size() - 1; ++i)
        {
            auto startPoint = points[i];
            auto endPoint = points[i + 1];
            
            auto startPos = normalizePointInBounds (startPoint.point());
            auto endPos = normalizePointInBounds (endPoint.point());
            
            path.addLineSegment (juce::Line<float> (startPos, endPos), STROKE_WIDTH);
        }
    }
    
    g.setColour (STROKE_COLOUR);
    g.fillPath (path);
}

void ArchetypeView::resized()
{
    
}

ArchetypalGlyph ArchetypeView::getArchetype()
{
    return archetype;
}

void ArchetypeView::setArchetype (ArchetypalGlyph archetype)
{
    this->archetype = archetype;
    repaint();
}

void ArchetypeView::mouseDrag (const juce::MouseEvent &event)
{
    juce::DragAndDropContainer* dragC =
            juce::DragAndDropContainer::findParentDragContainerFor (this);
    if (! dragC->isDragAndDropActive())
    {
        dragC->startDragging("TargetSouce", this);
    }
}

juce::Point<float> ArchetypeView::normalizePointInBounds (juce::Point<float> point)
{
    float padding = 10.0f;
    float xScaled = (point.x + 1.0f) / 2.0f;
    float yScaled = (-point.y + 1.0f) / 2.0f;
    
    float xInBounds = padding + xScaled * (getWidth() - padding * 2.0f);
    float yInBounds = padding + yScaled * (getHeight() - padding * 2.0f);
    
    return { xInBounds, yInBounds };
}