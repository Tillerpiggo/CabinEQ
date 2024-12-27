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
    drawGridLines (g);
    drawGlyphs (g);
    drawCenterDots (g);
    drawPlayingDots (g);
    drawDraggingGlyph (g);
    drawSelection (g);
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
    // If not hovering, begin a selection
    if (hoveringId == -1)
    {
        auto eventPos = event.getPosition().toFloat();
        selectionStart = eventPos;
        selectionEnd = eventPos;
        selectionRect = { eventPos, eventPos };
        selectedIds.clear();
        return;
    }
    
    selectionStart.reset();
    selectionEnd.reset();
    selectionRect.reset();
    
    // If holding ctrl, start dragging it as a duplicate
    if ((event.mods.isCtrlDown() || event.mods.isAltDown()) && hoveringId != -1)
    {
        // TODO: copy entire selection - doesn't seem necessary for now
        selectedIds.clear();
        
        for (const auto& glyph : glyphs)
        {
            if (glyph.getId() == hoveringId)
            {
                draggingGlyph = glyph.getArchetype();
                draggingPos = event.getPosition().toFloat();
                draggingSize = glyph.getSizeFactor();
                isDraggingDuplicate = true;
                return;
            }
        }
        // If we weren't hovering over anything, we can probably proceed as usual (?)
    }
    
    // Drag whatever we're hovering over
    draggingId = hoveringId;
    if (draggingId == -1)
        return;
    
    // If we right click, delete anything selected
    if (event.mods.isRightButtonDown())
    {
        removeGlyph (hoveringId);
        if (selectedIds.find (hoveringId) != selectedIds.end())
            for (const auto& selectedId : selectedIds)
                removeGlyph (selectedId);
    }
    
    // If we selected multiple, register their starting positions
    else if (selectedIds.size() > 0)
    {
        // Save the starting position of every selected glyph
        for (const auto& glyph : glyphs)
        {
            if (selectedIds.find (glyph.getId()) != selectedIds.end())
            {
                auto startingPos = getLocalCenterPosForGlyph (glyph);
                selectedIdToStartingPosition[glyph.getId()] = startingPos;
                if (glyph.getId() == hoveringId)
                selectionStartPos = event.getPosition().toFloat();
            }
        }
        
        moveSelectedGlyphsToMouseEvent (event);
    }
    
    // If we're just dragging one, try to move it
    else
    {
        moveGlyph (draggingId, getNormalizedPointFromMouseEvent (event));
    }
}

void GlyphGridView::mouseDrag (const juce::MouseEvent &event)
{
    if (selectionStart.has_value() && selectionEnd.has_value())
    {
        selectionEnd = event.getPosition().toFloat();
        
        // Get constants for selection
        float xMin = fmin (selectionStart->x, selectionEnd->x);
        float xMax = fmax (selectionStart->x, selectionEnd->x);
        float yMin = fmin (selectionStart->y, selectionEnd->y);
        float yMax = fmax (selectionStart->y, selectionEnd->y);
        selectionRect = { xMin, yMin, xMax - xMin, yMax - yMin };
        
        selectedIds.clear();
        for (const auto& glyph : glyphs)
        {
            auto pos = getLocalCenterPosForGlyph (glyph);
            if (selectionRect->contains (pos))
            {
                selectedIds.insert (glyph.getId());
            }
        }
    }
    else if (selectedIds.size() > 0)
    {
        moveSelectedGlyphsToMouseEvent (event);
    }
    else if (isDraggingDuplicate)
    {
        draggingPos = event.getPosition().toFloat();
    }
    else if (draggingId != -1)
    {
        moveGlyph (draggingId, getNormalizedPointFromMouseEvent (event));
    }
}

void GlyphGridView::mouseUp (const juce::MouseEvent &event)
{
    if (selectionStart.has_value() || selectionEnd.has_value())
    {
        selectionStart.reset();
        selectionEnd.reset();
        selectionRect.reset();
        // selection rect should stay intact until an unrelated mouse event occurs
    }
    else if (selectedIds.size() > 0)
    {
        moveSelectedGlyphsToMouseEvent (event);
    }
    else if (isDraggingDuplicate)
    {
        dropDraggingGlyph();
        isDraggingDuplicate = false;
    }
    else if (draggingId != -1)
    {
        moveGlyph (draggingId, getNormalizedPointFromMouseEvent (event));
        draggingId = -1;
    }
}

void GlyphGridView::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    if (selectedIds.size() > 0)
    {
        scaleSelectedGlyphs (wheel.deltaY);
    }
    
    if (hoveringId == -1)
        return;
    
    if (event.mods.isShiftDown())
    {
        incrementVolume (hoveringId, wheel.deltaY);
    }
    else
    {
        incrementSizeFactor (hoveringId, wheel.deltaY);
    }
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
        draggingSize = 0.5f;
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
    draggingSize.reset();
    repaint();
}

void GlyphGridView::itemDropped (const SourceDetails& dragSourceDetails)
{
    dropDraggingGlyph();
}

bool GlyphGridView::shouldDrawDragImageWhenOver()
{
    return false;
}

void GlyphGridView::drawGridLines (juce::Graphics& g)
{
    // For now, just draw a border
    g.setColour (juce::Colours::blue);
    g.drawRect (0, 0, getWidth(), getHeight());
}

void GlyphGridView::drawGlyphs (juce::Graphics& g)
{
    for (const auto& glyph : glyphs)
    {
        bool isSelected = false;
        for (const int id : selectedIds)
            if (glyph.getId() == id)
                isSelected = true;
        drawGlyph (g, glyph.getStrokes(), glyph.getCenterPos(), glyph.getSizeFactor(), isSelected ? juce::Colours::white : STROKE_COLOUR.withAlpha (glyph.getVolume()));
    }
}

void GlyphGridView::drawCenterDots (juce::Graphics &g)
{
    for (const auto& glyph : glyphs)
    {
        auto centerPoint = getLocalCenterPosForGlyph (glyph);
        float dotRadius = (glyph.getId() == draggingId || glyph.getId() == hoveringId) ? DOT_RADIUS_DRAGGING : DOT_RADIUS_DEFAULT;
        drawDot (g, centerPoint, dotRadius, DOT_COLOUR);
    }
}

void GlyphGridView::drawPlayingDots (juce::Graphics& g)
{
    for (const auto& glyph : glyphs)
    {
        auto [point, _] = glyph.positionAtTime (dataSource->getCurrPlayingTime());
        point = getLocalPointFromNormalizedPoint (point, glyph.getCenterPos(), glyph.getSizeFactor());
        
        drawDot (g, point, DOT_RADIUS_PLAYING, PLAYING_DOT_COLOUR);
    }
}

void GlyphGridView::drawSelection (juce::Graphics& g)
{
    if (selectionRect.has_value())
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.fillRect (selectionRect.value());
        g.setColour (juce::Colours::white);
        g.drawRect (selectionRect.value());
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
        drawGlyph (g, draggingGlyph->getStrokes(), getNormalizedPointFromLocalPoint (draggingPos.value()), draggingSize.value(), STROKE_COLOUR.withAlpha (0.5f));
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

juce::Point<float> GlyphGridView::getLocalCenterPosForGlyph (const Glyph& glyph)
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
        auto glyphPos = getLocalCenterPosForGlyph (glyph);
        float dist = glyphPos.getDistanceFrom (hoverPos);
        if (dist < minDist)
        {
            minDist = dist;
            newHoveringId = glyph.getId();
        }
    }
    
    hoveringId = newHoveringId;
}

void GlyphGridView::dropDraggingGlyph()
{
    if (listener != nullptr)
    {
        listener->addGlyph (draggingGlyph.value(), getNormalizedPointFromLocalPoint (draggingPos.value()), draggingSize.value());
        glyphs = dataSource->getGlyphs();
    }
    
    draggingGlyph.reset();
    draggingPos.reset();
    draggingSize.reset();
    repaint();
}

void GlyphGridView::moveGlyph (int glyphId, juce::Point<float> centerPos)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->moveGlyph (glyphId, centerPos);
        glyphs = dataSource->getGlyphs();
    }
}

void GlyphGridView::moveSelectedGlyphsToMouseEvent (const juce::MouseEvent& event)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    // Assume selectionStartPos and selectedIdToStartingPosition are valid
    std::unordered_map<int, juce::Point<float>> idsToPositions;
    auto eventPos = event.getPosition().toFloat();
    float xMoved = eventPos.x - selectionStartPos.x;
    float yMoved = eventPos.y - selectionStartPos.y;
    
    for (const auto& pair : selectedIdToStartingPosition)
    {
        auto id = pair.first;
        auto startingPos = pair.second;
        juce::Point<float> currPos = { startingPos.x + xMoved, startingPos.y + yMoved };
        currPos = getNormalizedPointFromLocalPoint (currPos); // make sure to normalize
        idsToPositions[id] = currPos;
    }
    
    listener->moveGlyphs (idsToPositions);
    glyphs = dataSource->getGlyphs();
}

void GlyphGridView::removeGlyph (int glyphId)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->removeGlyph (glyphId);
        glyphs = dataSource->getGlyphs();
    }
}

void GlyphGridView::incrementVolume (int glyphId, float increment)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->incrementGlyphVolume (glyphId, increment);
        glyphs = dataSource->getGlyphs();
    }
}

void GlyphGridView::incrementSizeFactor (int glyphId, float increment)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->incrementSizeFactor (glyphId, increment);
        glyphs = dataSource->getGlyphs();
    }
}

void GlyphGridView::scaleSelectedGlyphs (float increment)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        listener->scaleGlyphs (selectedIds, increment);
        glyphs = dataSource->getGlyphs();
    }
}
