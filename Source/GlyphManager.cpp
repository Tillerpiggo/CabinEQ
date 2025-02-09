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
    ArchetypalGlyph starGlyph (2, {
        Stroke ({{ 0, 1 }, { -0.588, -0.809 }, { 0.951, 0.309 }, { -0.951, 0.309 }, { 0.588, -0.809 }, { 0, 1 }})
    });
    ArchetypalGlyph lineJulianGlyph (13, {
        Stroke ({{ -1, -1, 1 }, { 1, 1, 1 },  { -1, -1, 1 }})
    });
    ArchetypalGlyph horizontalGlyph (1, {
        Stroke ({{ -1, 0 }, { 1, 0 }, { -1, 0 }})
    });
    ArchetypalGlyph verticalGlyph (2, {
        Stroke ({{ 0, -1 }, { 0, 0 }}),
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 0, 0 }, { 0, 1 }}),
        Stroke ({{ 0, 1 }, { 0, 1 }}),
        Stroke ({{ 0, 1 }, { 0, 0 }}),
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 0, 0 }, { 0, -1 }}),
        Stroke ({{ 0, -1 }, { 0, -1 }})
    });
    
    ArchetypalGlyph verticalGlyph2 (10, {
        Stroke ({{ 0, -1 }, { 0, -0.33 }}), Stroke ({{ 0, -0.33 }, { 0, -0.33 }}),
        Stroke ({{ 0, -0.33 }, { 0, 0.33 }}), Stroke ({{ 0, 0.33 }, { 0, 0.33 }}),
        Stroke ({{ 0, 0.33 }, { 0, 1 }}), Stroke ({{ 0, 1 }, { 0, 1 }}),
        Stroke ({{ 0, 1 }, { 0, 0.33 }}), Stroke ({{ 0, 0.33 }, { 0, 0.33 }}),
        Stroke ({{ 0, 0.33 }, { 0, -0.33 }}), Stroke ({{ 0, -0.33 }, { 0, -0.33 }}),
        Stroke ({{ 0, -0.33 }, { 0, -1 }}), Stroke ({{ 0, -1 }, { 0, -1 }})
    });
    
    ArchetypalGlyph panningGlyph3 (11, {
        
        Stroke ({{ -1, 1 }, { -1, 1 }}),
        Stroke ({{ -1, 1 }, { -1, 0.33 }}), Stroke ({{ -1, 0.33 }, { -1, 0.33 }}),
        Stroke ({{ -1, 0.33 }, { -1, -0.33 }}), Stroke ({{ -1, -0.33 }, { -1, -0.33 }}),
        Stroke ({{ -1, -0.33 }, { -1, -1 }}), Stroke ({{ -1, -1 }, { -1, -1 }}),
        
        Stroke ({{ -0.5, 1 }, { -0.5, 1 }}),
        Stroke ({{ -0.5, 1 }, { -0.5, 0.33 }}), Stroke ({{ -0.5, 0.33 }, { -0.5, 0.33 }}),
        Stroke ({{ -0.5, 0.33 }, { -0.5, -0.33 }}), Stroke ({{ -0.5, -0.33 }, { -0.5, -0.33 }}),
        Stroke ({{ -0.5, -0.33 }, { -0.5, -1 }}), Stroke ({{ -0.5, -1 }, { -0.5, -1 }}),
        
        Stroke ({{ 0, 1 }, { 0, 1 }}),
        Stroke ({{ 0, 1 }, { 0, 0.33 }}), Stroke ({{ 0, 0.33 }, { 0, 0.33 }}),
        Stroke ({{ 0, 0.33 }, { 0, -0.33 }}), Stroke ({{ 0, -0.33 }, { 0, -0.33 }}),
        Stroke ({{ 0, -0.33 }, { 0, -1 }}), Stroke ({{ 0, -1 }, { 0, -1 }}),
        
        Stroke ({{ 0.5, 1 }, { 0.5, 1 }}),
        Stroke ({{ 0.5, 1 }, { 0.5, 0.33 }}), Stroke ({{ 0.5, 0.33 }, { 0.5, 0.33 }}),
        Stroke ({{ 0.5, 0.33 }, { 0.5, -0.33 }}), Stroke ({{ 0.5, -0.33 }, { 0.5, -0.33 }}),
        Stroke ({{ 0.5, -0.33 }, { 0.5, -1 }}), Stroke ({{ 0.5, -1 }, { 0.5, -1 }}),
        
        Stroke ({{ 1, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 1, 0.33 }}), Stroke ({{ 1, 0.33 }, { 1, 0.33 }}),
        Stroke ({{ 1, 0.33 }, { 1, -0.33 }}), Stroke ({{ 1, -0.33 }, { 1, -0.33 }}),
        Stroke ({{ 1, -0.33 }, { 1, -1 }}), Stroke ({{ 1, -1 }, { 1, -1 }})
    });
    
//    ArchetypalGlyph verticalGlyph3 (11, {
//        
//    });
    
//    ArchetypalGlyph diagonalGlyph (3, {
//        Stroke ({{ -1, -1 }, { 1, 1 }, { -1, -1 }}),
//    });
    
//    ArchetypalGlyph diagonalGlyph (3, {
//        Stroke ({{ -1, -1 }, { 0, 0 }}),
//        Stroke ({{ 0, 0 }, { 0, 0 }}),
//        Stroke ({{ 0, 0 }, { 1, 1 }}),
//        Stroke ({{ 1, 1 }, { 1, 1 }}),
//        Stroke ({{ 1, 1 }, { 0, 0 }}),
//        Stroke ({{ 0, 0 }, { 0, 0 }}),
//        Stroke ({{ 0, 0 }, { -1, -1 }}),
//        Stroke ({{ -1, -1 }, { -1, -1 }})
//    });
    ArchetypalGlyph diagonalGlyph (3, {
        Stroke ({{ -1, -1 }, { 1, 1 }})
    });
    
    ArchetypalGlyph rows (4, {
        Stroke ({{ -1, 1 }, { 1, 1 }}),
        Stroke ({{ -1, 0.5 }, { 1, 0.5 }}),
        Stroke ({{ -1, 0 }, { 1, 0 }}),
        Stroke ({{ -1, -0.5 }, { 1, -0.5 }}),
        Stroke ({{ -1, -1 }, { 1, -1 }})
    });
    
    ArchetypalGlyph cols (5, {
        Stroke ({{ -1, 1 }, { -1, -1 }}),
        Stroke ({{ -0.5, 1 }, { -0.5, -1 }}),
        Stroke ({{ 0, 1 }, { 0, -1 }}),
        Stroke ({{ 0.5, 1 }, { 0.5, -1 }}),
        Stroke ({{ 1, 1 }, { 1, -1 }})
    });
    
    ArchetypalGlyph diagonalGlyph2 (4, {
        Stroke ({{ 1, -1 }, { -1, 1 }, { 1, -1 }})
//        Stroke ({{ 1, -1, 0.5 }, { -1, 1, 0.5 }, { 1, -1, 0.5 }})
    });
    
    ArchetypalGlyph steepDiagonalGlpyh (5, {
        Stroke ({{ -0.5, -1 }, { 0.5, 1 }, { -0.5, -1 }})
    });
    
    ArchetypalGlyph steepDiagonalGlyph2 (6, {
        Stroke ({{ 0.5, -1 }, { -0.5, 1 }, { 0.5, -1 }})
    });
    
    ArchetypalGlyph comparisonGlyph (6, {
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 1, 1 }, { 1, 1 }})
    });
    
    ArchetypalGlyph threeByThree (7, {
        Stroke ({{ -1, 1 }, { 0, 1 }}), Stroke ({{ 0, 1 }, { 0, 1 }}),
        Stroke ({{ 0, 1 }, { 1, 1 }}), Stroke ({{ 1, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 1, 0 }}), Stroke ({{ 1, 0 }, { 1, 0 }}),
        Stroke ({{ 1, 0 }, { 0, 0 }}), Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 0, 0 }, { -1, 0 }}), Stroke ({{ -1, 0 }, { -1, 0 }}),
        Stroke ({{ -1, 0 }, { -1, -1 }}), Stroke ({{ -1, -1 }, { -1, -1 }}),
        Stroke ({{ -1, -1 }, { 0, -1 }}), Stroke ({{ 0, -1 }, { 0, -1 }}),
        Stroke ({{ 0, -1 }, { 1, -1 }}), Stroke ({{ 1, -1 }, { 1, -1 }})
    });
    
    ArchetypalGlyph squareGrid (8, {
        Stroke ({{ -1, -1 }, { 1, 1 }}), Stroke ({{ 1, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 1, -1 }}), Stroke ({{ 1, -1 }, { 1, -1 }}),
        Stroke ({{ 1, -1 }, { -1, 1 }}), Stroke ({{ -1, 1 }, { -1, 1 }}),
        Stroke ({{ -1, 1 }, { -1, -1 }}), Stroke ({{ -1, -1 }, { -1, -1 }})
    });
    
    ArchetypalGlyph longerDiagonal (9, {
        Stroke ({{ -1, -1 }, { -0.33, -0.33 }}), Stroke ({{ -0.33, -0.33 }, { -0.33, -0.33 }}),
        Stroke ({{ -0.33, -0.33 }, { 0.33, 0.33 }}), Stroke ({{ 0.33, 0.33 }, { 0.33, 0.33 }}),
        Stroke ({{ 0.33, 0.33 }, { 1, 1 }}), Stroke ({{ 1, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 0.33, 0.33 }}), Stroke ({{ 0.33, 0.33 }, { 0.33, 0.33 }}),
        Stroke ({{ 0.33, 0.33 }, { -0.33, -0.33 }}), Stroke ({{ -0.33, -0.33 }, { -0.33, -0.33 }}),
        Stroke ({{ -0.33, -0.33 }, { -1, -1 }}), Stroke ({{ -1, -1 }, { -1, -1 }})
    });
    
    ArchetypalGlyph diagonalGrid (8, {
        Stroke ({{ -1, 0 }, { 0, 1 }}),
        Stroke ({{ -1, -1 }, { 1, 1 }}),
        Stroke ({{ 0, -1 }, { 1, 0 }}),
    });
    
    ArchetypalGlyph threeByThreeSquished (7, {
        Stroke ({{ -1, 0.5 }, { 0, 0.5 }, { 1, 0.5 }}),
        Stroke ({{ -1, 0 }, { 0, 0 }, { 1, 0 }}),
        Stroke ({{ -1, -0.5 }, { 0, -0.5 }, { 1, -0.5 }})
    });
    
    ArchetypalGlyph diagonalDots (8, {
        Stroke ({{ -1, -1 }, { -1, -1 }}),
        Stroke ({{ -0.5, -0.5 }, { -0.5, -0.5 }}),
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 0.5, 0.5 }, { 0.5, 0.5 }}),
        Stroke ({{ 1, 1 }, { 1, 1 }}),
        Stroke ({{ 0.5, 0.5 }, { 0.5, 0.5 }}),
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ -0.5, -0.5 }, { -0.5, -0.5 }}),
    });
    
    ArchetypalGlyph growingDot (9, {
        Stroke ({{ 0, 0, 0.0 }, { 0, 0, 1 }, { 0, 0, 0.0 }})
    });
    
    ArchetypalGlyph growingDot2 (10, {
        Stroke ({{ 0, 0, 0.0 }, { 0, 0, 1 }, { 0, 0, 0.0 }, { 0, 0, 1 }, { 0, 0, 0 }})
    });
    ArchetypalGlyph circumscribedCircle (11, {
        Stroke ({
            { 1, 0 },
            { 0.981, 0.195 },
            { 0.924, 0.383 },
            { 0.831, 0.556 },
            { 0.707, 0.707 },
            { 0.556, 0.831 },
            { 0.383, 0.924 },
            { 0.195, 0.981 },
            { 0, 1 },
            { -0.195, 0.981 },
            { -0.383, 0.924 },
            { -0.556, 0.831 },
            { -0.707, 0.707 },
            { -0.831, 0.556 },
            { -0.924, 0.383 },
            { -0.981, 0.195 },
            { -1, 0 },
            { -0.981, -0.195 },
            { -0.924, -0.383 },
            { -0.831, -0.556 },
            { -0.707, -0.707 },
            { -0.556, -0.831 },
            { -0.383, -0.924 },
            { -0.195, -0.981 },
            { 0, -1 },
            { 0.195, -0.981 },
            { 0.383, -0.924 },
            { 0.556, -0.831 },
            { 0.707, -0.707 },
            { 0.831, -0.556 },
            { 0.924, -0.383 },
            { 0.981, -0.195 },
            { 1, 0 }
        })
    });
    
    ArchetypalGlyph heartShape (12, {
        Stroke ({
            { 0.000, -0.500 },
            { 0.300, -0.600 },
            { 0.600, -0.400 },
            { 0.800, -0.100 },
            { 0.700, 0.300 },
            { 0.400, 0.600 },
            { 0.000, 0.700 },
            { -0.400, 0.600 },
            { -0.700, 0.300 },
            { -0.800, -0.100 },
            { -0.600, -0.400 },
            { -0.300, -0.600 },
            { 0.000, -0.500 }
        })
    });
    ArchetypalGlyph commandSymbol (13, {
        Stroke ({
            { -0.33, 0.33 },
            { -0.33, 1 },
            { -1, 1 },
            { -1, 0.33 },
            { -0.33, 0.33 },
            { 0.33, 0.33 },
            { 1, 0.33 },
            { 1, 1 },
            { 0.33, 1 },
            { 0.33, 0.33 },
            { 0.33, -0.33 },
            { 0.33, -1 },
            { 1, -1 },
            { 1, -0.33 },
            { 0.33, -0.33 },
            { -0.33, -0.33 },
            { -1, -0.33 },
            { -1, -1 },
            { -0.33, -1 },
            { -0.33, -0.33 },
            { -0.33, 0.33 }
        })
    });
    
    std::vector<Stroke> strokes;
    float gridSize = 5;
    for (int col = 0; col < gridSize; col++)
    {
        for (int row = 0; row < gridSize; row++)
        {
        
            float startX = (2.0f * (float) row / (gridSize - 1)) - 1.0f;
            float startY = (2.0f * (float) (gridSize - col) / (gridSize - 1)) - 1.0f;
            float offset = 2.0f / (gridSize - 1);
            
            startX -= offset / 2.0f;
            startY -= offset / 2.0f;
            
            strokes.push_back (Stroke ({{ startX, startY } , { startX, startY - offset }, { startX + offset, startY - offset }, { startX + offset, startY }, { startX, startY }}));
        }
    }
    ArchetypalGlyph grid (50, {
        strokes
    });

    archetypalGlyphs.push_back (horizontalGlyph);
    archetypalGlyphs.push_back (verticalGlyph);
    archetypalGlyphs.push_back (verticalGlyph2);
    archetypalGlyphs.push_back (diagonalGlyph);
    archetypalGlyphs.push_back (rows);
    archetypalGlyphs.push_back (cols);
    archetypalGlyphs.push_back (longerDiagonal);
    archetypalGlyphs.push_back (panningGlyph3);
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
