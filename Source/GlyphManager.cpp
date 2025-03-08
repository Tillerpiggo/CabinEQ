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
    ArchetypalGlyph verticalGlyph (1, {
        Stroke ({{ 0, -1 }, { 0, 1 }, { 0, -1 }})
    });
    
    ArchetypalGlyph diagonalLeft (2, {
        Stroke ({{ -1, -1 }, { 1, 1 }, { -1, -1 }})
    });
    
    ArchetypalGlyph diagonalRight (3, {
        Stroke ({{ 1, -1 }, { -1, 1 }, { 1, -1 }})
    });
    
    archetypalGlyphs.push_back (verticalGlyph);
    archetypalGlyphs.push_back (diagonalLeft);
    archetypalGlyphs.push_back (diagonalRight);
}

void GlyphManager::addGlyph (int archetypeId, juce::Point<float> centerPos)
{
    // Try to find archetypeId in the list of archetypalGlyphs (TODO: ensure the list is ordered by id so that we can just do simple array access here)
    std::optional<ArchetypalGlyph> archetypeWithId;
    for (const auto& archetype : archetypalGlyphs)
    {
        if (archetype.getId() == archetypeId)
        {
            archetypeWithId = archetype;
            break;
        }
    }
    if (! archetypeWithId.has_value())
        return;
    
    // Add glyph with next available id
    glyphs.push_back (Glyph (nextAvailableId, archetypeWithId.value()));
    glyphs[glyphs.size() - 1].setCenterPos (centerPos);
    
    updateNextAvailableId();
}

void GlyphManager::addGlyph (ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor)
{
    glyphs.push_back (Glyph (nextAvailableId, archetype, centerPos, sizeFactor));
    updateNextAvailableId();
}

void GlyphManager::moveGlyph (int glyphId, juce::Point<float> centerPos)
{
    for (int i = 0; i < glyphs.size(); ++i)
    {
        if (glyphs[i].getId() == glyphId)
        {
            glyphs[i].setCenterPos (centerPos);
            break;
        }
    }
}

void GlyphManager::removeGlyph (int glyphId)
{
    // Find the glyph with that id, and if it exists, remove it
    for (int i = 0; i < glyphs.size(); ++i)
    {
        if (glyphs[i].getId() == glyphId)
        {
            glyphs.erase (glyphs.begin() + i);
            break;
        }
    }
    
    updateNextAvailableId();
}

void GlyphManager::incrementGlyphVolume (int glyphId, float increment)
{
    // Find the glyph with that id, and if it exists, increment the size factor
    for (int i = 0; i < glyphs.size(); ++i)
    {
        if (glyphs[i].getId() == glyphId)
        {
            glyphs[i].incrementVolume (increment);
            break;
        }
    }
}

void GlyphManager::incrementSizeFactor (int glyphId, float horizontalIncrement, float verticalIncrement)
{
    // Find the glyph with that id, and if it exists, increment the size factor
    for (int i = 0; i < glyphs.size(); ++i)
    {
        if (glyphs[i].getId() == glyphId)
        {
            glyphs[i].incrementSizeFactor (horizontalIncrement, verticalIncrement);
            break;
        }
    }
}

void GlyphManager::moveGlyphs (std::unordered_map<int, juce::Point<float>>& idsToPositions)
{
    // Find max distance we can move the glyphs
    float maxX = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::max();
    for (const auto& glyph : glyphs)
    {
        // If the glyph is in the given position map
        if (idsToPositions.find (glyph.getId()) != idsToPositions.end())
        {
            auto [moveX, moveY] = glyph.getProjectedMoveDistance (idsToPositions[glyph.getId()]);
            
            if (abs (moveX) < abs (maxX))
                maxX = moveX;
            if (abs (moveY) < abs (maxY))
                maxY = moveY;
        }
    }
    
    for (auto& glyph : glyphs)
    {
        if (idsToPositions.find (glyph.getId()) != idsToPositions.end())
        {
            glyph.moveBy ({ maxX, maxY });
        }
    }
}

void GlyphManager::scaleGlyphs (std::unordered_set<int> glyphIds, float increment)
{
    float maxIncrement = std::numeric_limits<float>::max();
    for (const auto& glyph : glyphs)
    {
        if (glyphIds.find (glyph.getId()) != glyphIds.end())
        {
            auto projectedIncrement = glyph.getProjectedSizeFactorIncrement (increment);
            if (abs (projectedIncrement) < abs (maxIncrement))
                maxIncrement = projectedIncrement;
        }
    }
    
    for (auto& glyph : glyphs)
    {
        if (glyphIds.find (glyph.getId()) != glyphIds.end())
        {
            glyph.incrementSizeFactor (maxIncrement, maxIncrement);
        }
    }
}

void GlyphManager::addArchetypalGlyphs (std::vector<ArchetypalGlyph> newGlyphs)
{
    for (const auto& glyph : newGlyphs)
        archetypalGlyphs.push_back (glyph);
}
void GlyphManager::addArchetypalGlyph (ArchetypalGlyph glyph)
{
    archetypalGlyphs.push_back (glyph);
}

void GlyphManager::setIsCascading (bool isCascading)
{
    for (auto& glyph : glyphs)
        glyph.setIsCascading (isCascading);
}

void GlyphManager::setDensity (int density)
{
    for (auto& glyph : glyphs)
        glyph.setDensity (density);
}

void GlyphManager::setStrokeOverlap (float strokeOverlap)
{
    for (auto& glyph : glyphs)
        glyph.setStrokeOverlap (strokeOverlap);
}

void GlyphManager::setDotOverlap (float dotOverlap)
{
    for (auto& glyph : glyphs)
        glyph.setDotOverlap (dotOverlap);
}

void GlyphManager::setRampLength (float rampLength)
{
    for (auto& glyph : glyphs)
        glyph.setRampLength (rampLength);
}

const std::vector<Glyph>& GlyphManager::getGlyphs()
{
    return glyphs;
}

const std::vector<ArchetypalGlyph>& GlyphManager::getArchetypalGlyphs()
{
    return archetypalGlyphs;
}

void GlyphManager::updateNextAvailableId()
{
    if (glyphs.size() == 0)
    {
        nextAvailableId = 0;
        return;
    }
    
    // Get a sorted list of ids
    std::vector<int> ids;
    for (const auto& glyph : glyphs)
        ids.push_back (glyph.getId());
    
    if (ids.size() == 0)
    {
        nextAvailableId = 0;
        return;
    }
    
    std::sort (ids.begin(), ids.end());
    
    // Find the next available id
    int nextId = 0;
    for (const int id : ids)
        if (nextId == id)
            nextId++;
    nextAvailableId = nextId;
}