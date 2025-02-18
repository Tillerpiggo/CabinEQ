/*
  ==============================================================================

    CheckerboardView.cpp
    Created: 10 Feb 2025 2:58:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CheckerboardView.h"

CheckerboardView::CheckerboardView()
{
    startTimer (5);
}

CheckerboardView::~CheckerboardView()
{
    
}

void CheckerboardView::paint (juce::Graphics& g)
{
    drawSquares (g);
    drawGridLines (g);
    drawSoloBorder (g);
}

void CheckerboardView::resized()
{
    
}

void CheckerboardView::mouseMove (const juce::MouseEvent &event)
{
    // Update hoverSquareCoords
    hoverSquareCoords = coordsForMouseEvent (event);
}

void CheckerboardView::mouseDown (const juce::MouseEvent &event)
{
    mouseDownCoords = coordsForMouseEvent (event);
    if (! hasSelection)
    {
        soloSquareCoords.clear();
        soloSquareCoords.insert (mouseDownCoords.value());
//        selectBetween (mouseDownCoords.value(), mouseDownCoords.value());
    }
}

void CheckerboardView::mouseDrag (const juce::MouseEvent &event)
{
    // Update solosquarecoords live to be the area between mouseDrag and mouseDown
    auto mouseDragCoords = coordsForMouseEvent (event);
    if (! mouseDownCoords.has_value() || ! mouseDragCoords.has_value())
        return;
    
    if (! hasSelection)
    {
        selectBetween (mouseDownCoords.value(), mouseDragCoords.value());
    }
}

void CheckerboardView::mouseUp (const juce::MouseEvent &event)
{
    auto mouseUpCoords = coordsForMouseEvent (event);
    
    // If we clicked on something and didn't move our mouse to a different square, toggle the solo at that point
//    if (hoverSquareCoords.has_value() && mouseUpCoords == hoverSquareCoords)
//    {
//        if (soloSquareCoords.find (mouseUpCoords.value()) != soloSquareCoords.end())
//            soloSquareCoords.erase (mouseUpCoords.value());
//        else
//            soloSquareCoords.insert (mouseUpCoords.value());
//        
//        if (listener != nullptr)
//            listener->setSoloSquareCoords (soloSquareCoords);
//    }
    
    if (mouseUpCoords.has_value() && mouseDownCoords.has_value())
    {
        if (! hasSelection)
        {
            selectBetween (mouseDownCoords.value(), mouseUpCoords.value());
        }
        else
        {
            soloSquareCoords.clear();
            listener->setSoloSquareCoords (soloSquareCoords);
        }
        hasSelection = ! hasSelection;
    }
    
    mouseDownCoords = std::nullopt;
}

void CheckerboardView::mouseExit (const juce::MouseEvent &event)
{
    hoverSquareCoords.reset();
}

void CheckerboardView::setListener (CheckerboardViewListener* listener)
{
    this->listener = listener;
}

void CheckerboardView::setDataSource (CheckerboardViewDataSource* dataSource)
{
    this->dataSource = dataSource;
}

void CheckerboardView::updateIsPlaying()
{
    if (dataSource != nullptr)
    {
        this->isPlaying = dataSource->getIsPlaying();
        repaint();
    }
}

void CheckerboardView::updateCheckerboard()
{
    if (dataSource != nullptr)
    {
        this->checkerboard = dataSource->getCheckerboard();
        repaint();
    }
}

void CheckerboardView::timerCallback()
{
    repaint();
    
//    if (soloCounter > soloMax)
//    {
//        isSolod = ! isSolod;
//        listener->setSoloSquareCoords (isSolod ? std::set<std::pair<int, int>>() : soloSquareCoords);
//        soloCounter = 0;
//    }
//    soloCounter++;
}

void CheckerboardView::drawGridLines (juce::Graphics& g)
{
    // Draw border
    g.setColour (BORDER_COLOUR);
    g.drawRect (0, 0, getWidth(), getHeight());
    
    // Draw grid lines
    g.setColour (GRIDLINE_COLOUR);
    auto [numRows, numCols] = checkerboard.getGridDimensions();
    int numHorizontalLines = numRows;
    int numVerticalLines = numCols;
    for (int i = 1; i < numHorizontalLines; ++i)
    {
        float y = ((float) i / (float) numHorizontalLines) * getHeight();
        g.drawLine (0, y, getWidth(), y);
    }
    for (int i = 1; i < numVerticalLines; ++i)
    {
        float x = ((float) i / (float) numVerticalLines) * getWidth();
        g.drawLine (x, 0, x, getHeight());
    }
}

void CheckerboardView::drawSoloBorder (juce::Graphics& g)
{
    // Finally draw a border around all solo'd squares
    juce::Path soloPath;
    g.setColour (SOLO_BORDER_COLOUR);
    float lineThickness = 4.0f;
    if (soloSquareCoords.empty()) // draw around everything if nothing is solo'd
    {
//        soloPath.addRectangle (0, 0, getWidth(), getHeight());
//        lineThickness *= 2;
    }
    else
    {
        auto [numRows, numCols] = checkerboard.getGridDimensions();
        
        // Iterate over each edge. If the edge has a neighbor, add it. Use a set of edges to avoid duplication
        for (const auto& coords : soloSquareCoords)
        {
            // Compute coords
            std::optional<std::pair<int, int>> topCoords;
            std::optional<std::pair<int, int>> bottomCoords;
            std::optional<std::pair<int, int>> leftCoords;
            std::optional<std::pair<int, int>> rightCoords;
            
            if (coords.first < numRows - 1) // freqIdx
                topCoords = { coords.first + 1, coords.second };
            if (coords.first > 0) // freqIdx
                bottomCoords = { coords.first - 1, coords.second };
            if (coords.second < numCols - 1) // panIdx
                rightCoords = { coords.first, coords.second + 1 };
            if (coords.second > 0)
                leftCoords = { coords.first, coords.second - 1 };
            
            // Make them optional if they aren't solo'd
            if (topCoords.has_value() && soloSquareCoords.find (topCoords.value()) != soloSquareCoords.end())
                topCoords = std::nullopt;
            if (bottomCoords.has_value() && soloSquareCoords.find (bottomCoords.value()) != soloSquareCoords.end())
                bottomCoords = std::nullopt;
            if (leftCoords.has_value() && soloSquareCoords.find (leftCoords.value()) != soloSquareCoords.end())
                leftCoords = std::nullopt;
            if (rightCoords.has_value() && soloSquareCoords.find (rightCoords.value()) != soloSquareCoords.end())
                rightCoords = std::nullopt;
            
            // If they're an absolute border, add them back with placeholder value so it renders
            if (coords.first == 0)
                bottomCoords = { 0, 0 };
            if (coords.first == numRows - 1)
                topCoords = { 0, 0 };
            if (coords.second == 0)
                leftCoords = { 0, 0 };
            if (coords.second == numCols - 1)
                rightCoords = { 0, 0 };
            
            juce::Rectangle<float> rect = getRectForFreqIdxAndPanIdx (coords.first, coords.second);
            
            // Top edge
            if (topCoords.has_value())
            {
                juce::Line<float> topEdge = { rect.getTopLeft().translated (lineThickness / 2.0f, lineThickness / 2.0f), rect.getTopRight().translated (-lineThickness / 2.0f, lineThickness / 2.0f) };
                soloPath.addLineSegment (topEdge, lineThickness / 2.0f);
            }
            
            // Bottom edge
            if (bottomCoords.has_value())
            {
                juce::Line<float> bottomEdge = { rect.getBottomLeft().translated (lineThickness / 2.0f, -lineThickness / 2.0f), rect.getBottomRight().translated (-lineThickness / 2.0f, -lineThickness / 2.0f) };
                soloPath.addLineSegment (bottomEdge, lineThickness);
            }
            
            // Left edge
            if (leftCoords.has_value())
            {
                juce::Line<float> leftEdge = { rect.getTopLeft().translated (lineThickness / 2.0f, lineThickness / 2.0f), rect.getBottomLeft().translated (lineThickness / 2.0f, -lineThickness / 2.0f) };
                soloPath.addLineSegment (leftEdge, lineThickness);
            }
            
            // Right edge
            if (rightCoords.has_value())
            {
                juce::Line<float> rightEdge = { rect.getTopRight().translated (-lineThickness / 2.0f, lineThickness / 2.0f), rect.getBottomRight().translated (-lineThickness / 2.0f, -lineThickness / 2.0f) };
                soloPath.addLineSegment (rightEdge, lineThickness);
            }
        }
    }
    g.strokePath (soloPath, juce::PathStrokeType (lineThickness));
}

void CheckerboardView::selectBetween (std::pair<int, int> point1, std::pair<int, int> point2)
{
    int minFreqIdx = std::min (point1.first, point2.first);
    int maxFreqIdx = std::max (point1.first, point2.first);
    int minPanIdx = std::min (point1.second, point2.second);
    int maxPanIdx = std::max (point1.second, point2.second);

    if (! soloSquareCoords.empty())
    {
        soloSquareCoords.clear();
        for (int freqIdx = minFreqIdx; freqIdx <= maxFreqIdx; freqIdx++)
        {
            for (int panIdx = minPanIdx; panIdx <= maxPanIdx; panIdx++)
            {
                soloSquareCoords.insert ({ freqIdx, panIdx });
            }
        }
    }
    if (listener != nullptr)
    {
        listener->setSoloSquareCoords (soloSquareCoords);
    }
}

void CheckerboardView::drawSquares (juce::Graphics& g)
{
    bool isPlaying = false;
    if (dataSource != nullptr)
        isPlaying = dataSource->getIsPlaying();
    
    auto grid = checkerboard.getGrid();
    auto [numRows, numCols] = checkerboard.getGridDimensions();
    for (int freqIdx = 0; freqIdx < numRows; ++freqIdx)
    {
        for (int panIdx = 0; panIdx < numCols; ++panIdx)
        {
            juce::Colour squareColour = isPlaying ? SQUARE_ON_COLOUR : SQUARE_OFF_COLOUR;
            bool isSoloSquare = soloSquareCoords.find ({ freqIdx, panIdx }) != soloSquareCoords.end();
//            bool isHoveringSquare = hoverSquareCoords.has_value() && freqIdx == hoverSquareCoords->first && panIdx == hoverSquareCoords->second;
            if (! isSoloSquare && ! soloSquareCoords.empty())
                squareColour = SQUARE_OFF_COLOUR;
            if (! grid[freqIdx][panIdx])
                squareColour = SQUARE_EMPTY_COLOUR;
//            if (! soloSquareCoords.empty())
//            {
//                squareColour = isSoloSquare ? squareColour : squareColour.withMultipliedAlpha (0.5f);
//            }
            
//            if (isHoveringSquare)
//                squareColour = squareColour.interpolatedWith (HOVER_SQUARE_COLOUR, grid[freqIdx][panIdx] ? 0.3f : 0.1f);
            drawSquare (freqIdx, panIdx, squareColour, g);
        }
    }
    
    // Now draw in all solod squares with a thin white selection color
    if (hoverSquareCoords.has_value() && soloSquareCoords.find (hoverSquareCoords.value()) == soloSquareCoords.end())
    {
        drawSquare (hoverSquareCoords->first, hoverSquareCoords->second, HOVER_SQUARE_COLOUR, g);
    }
    
//    // Finally draw a border around all solo'd squares
//    juce::Path soloPath;
//    g.setColour (SOLO_BORDER_COLOUR);
//    float lineThickness = 4.0f;
//    if (soloSquareCoords.empty()) // draw around everything if nothing is solo'd
//    {
//        soloPath.addRectangle (0, 0, getWidth(), getHeight());
//        lineThickness *= 2;
//    }
//    else
//    {
//        auto [numRows, numCols] = checkerboard.getGridDimensions();
//        
//        // Iterate over each edge. If the edge has a neighbor, add it. Use a set of edges to avoid duplication
//        for (const auto& coords : soloSquareCoords)
//        {
//            // Compute coords
//            std::optional<std::pair<int, int>> topCoords;
//            std::optional<std::pair<int, int>> bottomCoords;
//            std::optional<std::pair<int, int>> leftCoords;
//            std::optional<std::pair<int, int>> rightCoords;
//            
//            if (coords.first < numRows - 1) // freqIdx
//                topCoords = { coords.first + 1, coords.second };
//            if (coords.first > 0) // freqIdx
//                bottomCoords = { coords.first - 1, coords.second };
//            if (coords.second < numCols - 1) // panIdx
//                rightCoords = { coords.first, coords.second + 1 };
//            if (coords.second > 0)
//                leftCoords = { coords.first, coords.second - 1 };
//            
//            // Make them optional if they aren't solo'd
//            if (topCoords.has_value() && soloSquareCoords.find (topCoords.value()) != soloSquareCoords.end())
//                topCoords = std::nullopt;
//            if (bottomCoords.has_value() && soloSquareCoords.find (bottomCoords.value()) != soloSquareCoords.end())
//                bottomCoords = std::nullopt;
//            if (leftCoords.has_value() && soloSquareCoords.find (leftCoords.value()) != soloSquareCoords.end())
//                leftCoords = std::nullopt;
//            if (rightCoords.has_value() && soloSquareCoords.find (rightCoords.value()) != soloSquareCoords.end())
//                rightCoords = std::nullopt;
//            
//            // If they're an absolute border, add them back with placeholder value so it renders
//            if (coords.first == 0)
//                bottomCoords = { 0, 0 };
//            if (coords.first == numRows - 1)
//                topCoords = { 0, 0 };
//            if (coords.second == 0)
//                leftCoords = { 0, 0 };
//            if (coords.second == numCols - 1)
//                rightCoords = { 0, 0 };
//            
//            juce::Rectangle<float> rect = getRectForFreqIdxAndPanIdx (coords.first, coords.second);
//            
//            // Top edge
//            if (topCoords.has_value())
//            {
//                juce::Line<float> topEdge = { rect.getTopLeft().translated (lineThickness / 2.0f, lineThickness / 2.0f), rect.getTopRight().translated (-lineThickness / 2.0f, lineThickness / 2.0f) };
//                soloPath.addLineSegment (topEdge, lineThickness / 2.0f);
//            }
//            
//            // Bottom edge
//            if (bottomCoords.has_value())
//            {
//                juce::Line<float> bottomEdge = { rect.getBottomLeft().translated (lineThickness / 2.0f, -lineThickness / 2.0f), rect.getBottomRight().translated (-lineThickness / 2.0f, -lineThickness / 2.0f) };
//                soloPath.addLineSegment (bottomEdge, lineThickness);
//            }
//            
//            // Left edge
//            if (leftCoords.has_value())
//            {
//                juce::Line<float> leftEdge = { rect.getTopLeft().translated (lineThickness / 2.0f, lineThickness / 2.0f), rect.getBottomLeft().translated (lineThickness / 2.0f, -lineThickness / 2.0f) };
//                soloPath.addLineSegment (leftEdge, lineThickness);
//            }
//            
//            // Right edge
//            if (rightCoords.has_value())
//            {
//                juce::Line<float> rightEdge = { rect.getTopRight().translated (-lineThickness / 2.0f, lineThickness / 2.0f), rect.getBottomRight().translated (-lineThickness / 2.0f, -lineThickness / 2.0f) };
//                soloPath.addLineSegment (rightEdge, lineThickness);
//            }
//        }
//    }
//    g.strokePath (soloPath, juce::PathStrokeType (lineThickness));
}

void CheckerboardView::drawSquare (int freqIdx, int panIdx, juce::Colour squareColour, juce::Graphics& g)
{
    auto rect = getRectForFreqIdxAndPanIdx (freqIdx, panIdx);
    g.setColour (squareColour);
    g.fillRect (rect);
}

juce::Rectangle<float> CheckerboardView::getRectForFreqIdxAndPanIdx (int freqIdx, int panIdx)
{
    auto [numRows, numCols] = checkerboard.getGridDimensions();
    if (freqIdx < 0 || freqIdx >= numRows)
    {
        std::cerr << "ERR: getting rect for freq idx and pan idx with freqIdx out of bounds (freqIdx: " << freqIdx << ", numRows: " << numRows << std::endl;
        return { 0, 0, 0, 0 };
    }
    
    if (panIdx < 0 || panIdx >= numCols)
    {
        std::cerr << "ERR: getting rect for freq idx and pan idx with panIdx out of bounds (panIdx: " << panIdx << ", numCols: " << numCols << std::endl;
        return { 0, 0, 0, 0 };
    }
    
    // Get width and height
    float width = (float) getWidth() / (float) numCols;
    float height = (float) getHeight() / (float) numRows;
    
    // Get x and y coords
    
    float normalizedX = (float) panIdx / (float) numCols;
    float normalizedY = (float) (freqIdx + 1) / (float) numRows; // +1 so that when we flip it upside, y starts at the top instead of the bottom
    float absX = normalizedX * (float) getWidth();
    float absY = (1.0f - normalizedY) * (float) getHeight();
    
    return { absX, absY, width, height };
}

std::optional<std::pair<int, int>> CheckerboardView::coordsForMouseEvent (const juce::MouseEvent &event)
{
    if (dataSource == nullptr)
        return std::nullopt;
    
    // Get absolute coords
    auto [numRows, numCols] = checkerboard.getGridDimensions();
    auto absCoords = event.getPosition().toFloat();
    float relativeX = (absCoords.x / (float) getWidth()) * numCols;
    float relativeY = (1.0f - (absCoords.y / (float) getHeight())) * numRows;
    int panIdx = std::floor (relativeX);
    int freqIdx = std::floor (relativeY);
    
    // Make sure we're within bounds
    panIdx = std::min (numCols - 1, std::max (0, panIdx));
    freqIdx = std::min (numRows - 1, std::max (0, freqIdx));
    
    return std::optional<std::pair<int, int>> ({ freqIdx, panIdx });
}
