/*
  ==============================================================================

    GlyphGridView.cpp
    Created: 22 Dec 2024 11:00:52pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphGridView.h"

GlyphGridView::GlyphGridView()
{
    startTimer (5);
}

GlyphGridView::~GlyphGridView()
{
    
}

void GlyphGridView::paint (juce::Graphics& g)
{
    drawGlyphs (g);
    drawCenterDots (g);
    drawDraggingGlyph (g);
}

void GlyphGridView::resized()
{
    
}

void GlyphGridView::setListener (GlyphViewListener* listener)
{
    this->listener = listener;
}

void GlyphGridView::setDataSource (GlyphViewDataSource* dataSource)
{
    this->dataSource = dataSource;
    glyphs = dataSource->getGlyphs();
}

void GlyphGridView::timerCallback()
{
    repaint();
}

void GlyphGridView::mouseMove (const juce::MouseEvent &event)
{
    updateHoveringStatus (event);
}

void GlyphGridView::mouseDown (const juce::MouseEvent &event)
{
    draggingId = hoveringId;
    if (draggingId != -1)
    {
        
    }
    
    moveGlyph (draggingId, getNormalizedPointFromMouseEvent (event));
    
    // If we right click and were hovering, delete the glyph
    if (hoveringId != -1 && event.mods.isRightButtonDown())
        removeGlyph (hoveringId);
}

void GlyphGridView::mouseDrag (const juce::MouseEvent &event)
{
    
}

void GlyphGridView::mouseUp (const juce::MouseEvent &event)
{
    
}

void GlyphGridView::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    
}

bool GlyphGridView::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    return true;
}

void GlyphGridView::itemDragEnter (const SourceDetails& dragSourceDetails)
{
    if (ArchetypeView* archetypeView = dynamic_cast<ArchetypeView*> (dragSourceDetails.sourceComponent.get()))
    {
        draggingGlyph = archetypeView->getArchetype();
        draggingPos = { (float) dragSourceDetails.localPosition.x, (float) dragSourceDetails.localPosition.y };
    }
    repaint();
}

void GlyphGridView::itemDragMove (const SourceDetails& dragSourceDetails)
{
    draggingPos = { (float) dragSourceDetails.localPosition.x, (float) dragSourceDetails.localPosition.y };
    repaint();
}

void GlyphGridView::itemDragExit (const SourceDetails& dragSourceDetails)
{
    draggingGlyph.reset();
    draggingPos.reset();
    repaint();
}

void GlyphGridView::itemDropped (const SourceDetails& dragSourceDetails)
{
    if (listener != nullptr)
    {
        listener->addGlyph (draggingGlyph.value(), getNormalizedPointFromLocalPoint (draggingPos.value()), 0.5f);
        glyphs = dataSource->getGlyphs();
        repaint();
    }
    
    draggingGlyph.reset();
    draggingPos.reset();
    repaint();
}

bool GlyphGridView::shouldDrawDragImageWhenOver()
{
    return false;
}

void GlyphGridView::drawGlyphs (juce::Graphics& g)
{
    for (const auto& glyph : glyphs)
    {
        drawGlyph (g, glyph.getStrokes(), glyph.getCenterPos(), glyph.getSizeFactor(), STROKE_COLOUR);
    }
}

void GlyphGridView::drawCenterDots (juce::Graphics &g)
{
    for (const auto& glyph : glyphs)
    {
        auto centerPoint = getLocalizedCenterPointForGlyph (glyph);
        float dotRadius = (glyph.getId() == draggingId || glyph.getId() == hoveringId) ? DOT_RADIUS_DEFAULT : DOT_RADIUS_DRAGGING;
        drawDot (g, centerPoint, dotRadius, DOT_COLOUR);
    }
}

void GlyphGridView::drawGlyph (juce::Graphics& g, const std::vector<Stroke>& strokes, juce::Point<float> centerPos, float sizeFactor, juce::Colour strokeColour)
{
    juce::Path path;
    for (const auto& stroke : strokes)
    {
        auto points = stroke.getPoints();
        for (int i = 0; i < points.size() - 1; ++i)
        {
            auto startPoint = points[i];
            auto endPoint = points[i + 1];
            
            startPoint = getLocalPointFromNormalizedPoint (startPoint, centerPos, sizeFactor);
            endPoint = getLocalPointFromNormalizedPoint (endPoint, centerPos, sizeFactor);
            
            path.addLineSegment (juce::Line<float> (startPoint, endPoint), STROKE_WIDTH);
        }
    }
    
    g.setColour (strokeColour);
    g.fillPath (path);
}

void GlyphGridView::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour)
{
    // Just draw the dot
    g.setColour (dotColour);
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

void GlyphGridView::drawDraggingGlyph (juce::Graphics& g)
{
    if (draggingGlyph.has_value())
    {
        drawGlyph (g, draggingGlyph->getStrokes(), getNormalizedPointFromLocalPoint (draggingPos.value()), 0.5f, STROKE_COLOUR.withAlpha (0.5f));
    }
}

juce::Point<float> GlyphGridView::getLocalPointFromNormalizedPoint (juce::Point<float> point, juce::Point<float> centerPos, float sizeFactor)
{
    float padding = 10.0f;
    
    // Scale according to sizeFactor and centerPos;
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

juce::Point<float> GlyphGridView::getNormalizedPointFromLocalPoint (juce::Point<float> point)
{
    // First, convert to be within [0, 1]
    float positiveX = point.x / getWidth();
    float positiveY = point.y / getHeight();
    
    // Now, move to be between [-1, 1]
    float normalizedX = 2.0f * positiveX - 1.0f;
    float normalizedY = 1.0f - 2.0f * positiveY; // flip y because y axis is upside down in graphics libraries
    
    return { normalizedX, normalizedY };
}

juce::Point<float> GlyphGridView::getLocalizedCenterPointForGlyph (const Glyph& glyph)
{
    return getLocalPointFromNormalizedPoint ({ 0.0f, 0.0f }, glyph.getCenterPos(), glyph.getSizeFactor());
}

juce::Point<float> GlyphGridView::getNormalizedPointFromMouseEvent (const juce::MouseEvent& event)
{
    return getNormalizedPointFromLocalPoint (event.getPosition().toFloat());
}

void GlyphGridView::updateHoveringStatus (const juce::MouseEvent &event)
{
    // Figure out which node, if any, we're hovering over
    int newHoveringId = -1;
    float minDist = HOVER_MIN_DIST;
    auto hoverPos = event.getPosition().toFloat();
    for (const auto& glyph : glyphs)
    {
        auto glyphPos = getLocalizedCenterPointForGlyph (glyph);
        float dist = glyphPos.getDistanceFrom (hoverPos);
        if (dist < minDist)
        {
            minDist = dist;
            newHoveringId = glyph.getId();
        }
    }
    
    hoveringId = newHoveringId;
}

void GlyphGridView::moveGlyph (int glyphId, juce::Point<float> centerPos)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->moveGlyph (glyphId, centerPos);
        glyphs = dataSource->getGlyphs();
    }
}

void GlyphGridView::removeGlyph (int glyphId)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->removeGlyph (glyphId);
        glyphs = dataSource->getGlyphs();
    }
}
