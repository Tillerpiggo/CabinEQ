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
}

void CheckerboardView::mouseDrag (const juce::MouseEvent &event)
{
    
}

void CheckerboardView::mouseUp (const juce::MouseEvent &event)
{
    auto mouseUpCoords = coordsForMouseEvent (event);
    
    // If we clicked on something and didn't move our mouse to a different square, toggle the solo at that point
    if (hoverSquareCoords.has_value() && mouseUpCoords == hoverSquareCoords)
    {
        if (soloSquareCoords.find (mouseUpCoords.value()) != soloSquareCoords.end())
            soloSquareCoords.erase (mouseUpCoords.value());
        else
            soloSquareCoords.insert (mouseUpCoords.value());
        
        if (listener != nullptr)
            listener->setSoloSquareCoords (soloSquareCoords);
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
            bool isHoveringSquare = hoverSquareCoords.has_value() && freqIdx == hoverSquareCoords->first && panIdx == hoverSquareCoords->second;
            if (! grid[freqIdx][panIdx])
                squareColour = SQUARE_EMPTY_COLOUR;
            if (isSoloSquare)
                squareColour = SOLO_SQUARE_COLOUR;
            if (isHoveringSquare)
                squareColour = squareColour.interpolatedWith (HOVER_SQUARE_COLOUR, grid[freqIdx][panIdx] ? 0.3f : 0.1f);
            drawSquare (freqIdx, panIdx, squareColour, g);
        }
    }
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
