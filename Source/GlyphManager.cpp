/*
  ==============================================================================

    GlyphManager.cpp
    Created: 13 Nov 2024 11:45:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphManager.h"

GlyphManager::GlyphManager (std::vector<Glyph> glyphs)
    : glyphs (glyphs)
{
}

Glyph GlyphManager::getCurrGlyph()
{
    auto currGlyph = glyphs[glyphIdx];
    return currGlyph;
}

void GlyphManager::goToNext()
{
    if (hasNext())
        glyphIdx++;
}

void GlyphManager::goToPrev()
{
    if (hasPrev())
        glyphIdx--;
}

bool GlyphManager::hasNext()
{
    return glyphIdx < glyphs.size() - 1;
}

bool GlyphManager::hasPrev()
{
    return glyphIdx > 0;
}

void GlyphManager::addGlyphs (std::vector<Glyph> newGlyphs)
{
    for (const auto& glyph : newGlyphs)
    {
        glyphs.push_back (glyph);
    }
}

void GlyphManager::addGlyph (Glyph glyph)
{
    glyphs.push_back (glyph);
}

void GlyphManager::setSizeFactor (float sizeFactor)
{
    if (sizeFactor <= 0)
    {
        std::cerr << "tried to set illegal size factor with value <= 0 (sizeFactor=" << sizeFactor << ")" << std::endl;
        return;
    }
    
    this->sizeFactor = std::min (sizeFactor, 1.0f);
    
    // Make sure we're still in bounds
    if (! isInBounds())
        moveGlyphWithinBounds();
    // If not, move the center position so that we are in bounds ^
}

void GlyphManager::setCenterPos (juce::Point<float> centerPos)
{
    this->centerPos = centerPos;
    moveGlyphWithinBounds();
}

float GlyphManager::getSizeFactor() const
{
    return sizeFactor;
}

juce::Point<float> GlyphManager::getCenterPos() const
{
    return centerPos;
}

bool GlyphManager::isInBounds() const
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    juce::Point<float> centerPos = getCenterPos();
    
    bool isInXBounds = centerPos.x >= minX && centerPos.x <= maxX;
    bool isInYBounds = centerPos.y >= minY && centerPos.y <= maxY;
    return isInXBounds && isInYBounds;
}

void GlyphManager::moveGlyphWithinBounds()
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    centerPos.x = std::min (std::max (centerPos.x, minX), maxX);
    centerPos.y = std::min (std::max (centerPos.y, minY), maxY);
}

std::pair<float, float> GlyphManager::getXBounds() const
{
    float minX = -1.0f + sizeFactor;
    float maxX = 1.0f - sizeFactor;
    return { minX, maxX };
}

std::pair<float, float> GlyphManager::getYBounds() const
{
    float minY = -1.0f + sizeFactor;
    float maxY = 1.0f - sizeFactor;
    return { minY, maxY };
}
