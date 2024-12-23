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
}

GlyphGridView::~GlyphGridView()
{
    
}

void GlyphGridView::paint (juce::Graphics& g)
{
    drawGlyphs (g);
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

bool GlyphGridView::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    return true;
}

void GlyphGridView::itemDragEnter (const SourceDetails& dragSourceDetails)
{
    std::cout << "item drag enter" << std::endl;
}

void GlyphGridView::itemDragMove (const SourceDetails& dragSourceDetails)
{
    std::cout << "item drag move" << std::endl;
}

void GlyphGridView::itemDragExit (const SourceDetails& dragSourceDetails)
{
    std::cout << "item drag exit" << std::endl;
}

void GlyphGridView::itemDropped (const SourceDetails& dragSourceDetails)
{
    std::cout << "item dropped" << std::endl;
}

bool GlyphGridView::shouldDrawDragImageWhenOver()
{
    return true;
}

void GlyphGridView::drawGlyphs (juce::Graphics& g)
{
    for (const auto& glyph : glyphs)
    {
        drawGlyph (g, glyph);
    }
}

void GlyphGridView::drawGlyph (juce::Graphics& g, const Glyph& glyph)
{
    juce::Path path;
    std::vector<Stroke> strokes = glyph.getStrokes();
    for (const auto& stroke : strokes)
    {
        auto points = stroke.getPoints();
        for (int i = 0; i < points.size() - 1; ++i)
        {
            auto startPoint = points[i];
            auto endPoint = points[i + 1];
            
            startPoint = getNormalizedPointInBounds (startPoint, glyph.getCenterPos(), glyph.getSizeFactor());
            endPoint = getNormalizedPointInBounds (endPoint, glyph.getCenterPos(), glyph.getSizeFactor());
            
            path.addLineSegment (juce::Line<float> (startPoint, endPoint), STROKE_WIDTH);
        }
    }
    
    g.setColour (STROKE_COLOUR);
    g.fillPath (path);
}

juce::Point<float> GlyphGridView::getNormalizedPointInBounds (juce::Point<float> point, juce::Point<float> centerPos, float sizeFactor)
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
