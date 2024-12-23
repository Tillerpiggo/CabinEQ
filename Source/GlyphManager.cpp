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
    
    updateNextAvailableId();
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

void GlyphManager::addArchetypalGlyphs (std::vector<ArchetypalGlyph> newGlyphs)
{
    for (const auto& glyph : newGlyphs)
        archetypalGlyphs.push_back (glyph);
}
void GlyphManager::addArchetypalGlyph (ArchetypalGlyph glyph)
{
    archetypalGlyphs.push_back (glyph);
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
    // Get a sorted list of ids
    std::vector<int> ids;
    for (const auto& glyph : glyphs)
        ids.push_back (glyph.getId());
    std::sort (ids.begin(), ids.end());
    
    // Find the next available id
    int nextId = 0;
    for (const int id : ids)
        if (nextId == id)
            nextId++;
    nextAvailableId = nextId;
}
