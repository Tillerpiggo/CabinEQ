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
    return glyphs[glyphIdx];
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
