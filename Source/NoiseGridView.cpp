/*
  ==============================================================================

    NoiseGridView.cpp
    Created: 14 Dec 2024 5:04:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "NoiseGridView.h"

NoiseGridView::NoiseGridView()
{
    startTimer (12);
}

NoiseGridView::~NoiseGridView()
{
    
}

void NoiseGridView::paint (juce::Graphics& g)
{
    drawSquares (g);
    drawSequences (g);
}

void NoiseGridView::resized()
{
    // we'll just draw everything for now, maybe add a wrapper class later
}

void NoiseGridView::mouseMove (const juce::MouseEvent &event)
{
    
}

void NoiseGridView::mouseDown (const juce::MouseEvent &event)
{
    auto mouseDownCoords = rowAndColFromMouseEvent (event);
    if (mouseDownCoords.has_value())
    {
        addingCoords.push_back (mouseDownCoords.value());
    }
}

void NoiseGridView::mouseDrag (const juce::MouseEvent &event)
{
    std::cout << "adding coords: " << std::endl;
    for (const auto& coords : addingCoords)
    {
        std::cout << "(row: " << coords.first << ", col: " << coords.second << ")" << std::endl;
    }
    
    // Make sure we're dragging in a legit spot
    auto mouseDragCoords = rowAndColFromMouseEvent (event);
    if (! mouseDragCoords.has_value() || addingCoords.empty())
        return;
    
    auto mouseRowAndCol = mouseDragCoords.value();
    auto prevRowAndCol = addingCoords[addingCoords.size() - 1];
    
    // If we backtrack, remove the last square
    if (addingCoords.size() > 1)
    {
        auto prevPrevRowAndCol = addingCoords[addingCoords.size() - 2];
        if (mouseRowAndCol.first == prevPrevRowAndCol.first && mouseRowAndCol.second == prevPrevRowAndCol.second)
        {
            addingCoords.pop_back();
        }
    }

    // If we go somewhere new and adjacent, add in the value
    bool isPositionNew = true;
    for (const auto& oldCoords : addingCoords)
    {
        if (mouseRowAndCol.first == oldCoords.first && mouseRowAndCol.second == oldCoords.second)
        {
            isPositionNew = false;
        }
    }
    
    if (isPositionNew && abs (mouseRowAndCol.first - prevRowAndCol.first) + abs (mouseRowAndCol.second - prevRowAndCol.second) == 1)
    {
        addingCoords.push_back (mouseRowAndCol);
    }
}

void NoiseGridView::mouseUp (const juce::MouseEvent &event)
{
    addingCoords.clear();
}

void NoiseGridView::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    
}


void NoiseGridView::timerCallback()
{
    repaint(); // this is inefficient but whatever, we'll optimize it later
}

void NoiseGridView::setListener (NoiseGridViewListener* listener)
{
    this->listener = listener;
    updateVisualConstants();
}

void NoiseGridView::setDataSource (NoiseGridViewDataSource* dataSource)
{
    this->dataSource = dataSource;
    updateVisualConstants();
}

void NoiseGridView::removeListener()
{
    this->listener = nullptr;
}

void NoiseGridView::removeDataSource()
{
    this->dataSource = nullptr;
}

void NoiseGridView::drawSquares (juce::Graphics& g)
{
    if (listener != nullptr && dataSource != nullptr)
    {
        auto [numRows, numCols] = dataSource->getNumRowsAndNumCols();
        
        // Calculate visual constants
        float totalHorizontalPadding = (numCols + 1) * padding;
        float totalVerticalPadding = (numRows + 1) * padding;
        
        updateVisualConstants();
        
        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numCols; col++)
            {
                // draw a square centered at the row and col
                drawSquareAt (g, row, col, juce::Colours::white);
            }
        }
    }
}

void NoiseGridView::drawSquareAt (juce::Graphics& g, int row, int col, juce::Colour colour)
{
    auto [x, y] = squareCoordsFromRowAndCol (row, col);
    juce::Rectangle<float> square (x, y, squareSize, squareSize);
    g.setColour (colour);
    g.fillRect (square);
}

void NoiseGridView::drawSequences (juce::Graphics& g)
{
    // TODO
    
    // Draw the adding sequence...
    if (addingCoords.size() > 0)
    {
        juce::Path path;
        path.startNewSubPath (centerSquarePointFromCoords (addingCoords[0]));
        for (int i = 1; i < addingCoords.size(); ++i)
        {
            drawSquareAt (g, addingCoords[i].first, addingCoords[i].second, juce::Colours::lightblue);
            path.lineTo (centerSquarePointFromCoords (addingCoords[i]));
        }
        
//        g.setColour (juce::Colours::lightblue);
//        g.strokePath (path, juce::PathStrokeType (squareSize / 1.6f));
//        
        drawSquareAt (g, addingCoords[0].first, addingCoords[0].second, juce::Colours::blue);
    }
}

juce::Point<float> NoiseGridView::squareCoordsFromRowAndCol (int row, int col)
{
    float x = xOffset + padding + col * (squareSize + padding);
    float y = yOffset + padding + row * (squareSize + padding);
    return { x, y };
}

juce::Point<float> NoiseGridView::centerSquarePointFromCoords (std::pair<int, int> coords)
{
    juce::Point<float> squareTopLeft = squareCoordsFromRowAndCol (coords.first, coords.second);
    return { squareTopLeft.x + squareSize / 2.0f, squareTopLeft.y + squareSize / 2.0f };
}

std::optional<std::pair<int, int>> NoiseGridView::rowAndColFromMouseEvent (const juce::MouseEvent& event)
{
    // Get mouse coordinates relative to the component
    float mouseX = static_cast<float>(event.x);
    float mouseY = static_cast<float>(event.y);

    // Adjust for offsets
    float adjustedX = mouseX - xOffset - padding;
    float adjustedY = mouseY - yOffset - padding;

    // The size of each cell including padding
    float cellSize = squareSize + padding;

    // Calculate the potential column and row
    int col = static_cast<int>(adjustedX / cellSize);
    int row = static_cast<int>(adjustedY / cellSize);

    // Get the total number of rows and columns
    int numRows = 0;
    int numCols = 0;
    if (dataSource != nullptr)
    {
        std::tie(numRows, numCols) = dataSource->getNumRowsAndNumCols();
    }

    // Check if the calculated row and col are within the grid bounds
    if (row >= 0 && row < numRows && col >= 0 && col < numCols)
    {
        // Calculate the position of the square
        float squareX = xOffset + padding + col * cellSize;
        float squareY = yOffset + padding + row * cellSize;

        // Check if the mouse is within the square (excluding padding)
        if (mouseX >= squareX && mouseX <= squareX + squareSize &&
            mouseY >= squareY && mouseY <= squareY + squareSize)
        {
            std::cout << "calculated row and col (row: " << row << ", col: " << col << std::endl;
            return std::pair<int, int> { row, col };
        }
    }
    
    std::cout << "No proper row and col for mouse event :(" << std::endl;

    // If not within any square, return invalid indices
    return std::nullopt;
}

void NoiseGridView::updateVisualConstants()
{
    float width = getWidth();
    float height = getHeight();
    
    if (listener != nullptr && dataSource != nullptr)
    {
        auto [numRows, numCols] = dataSource->getNumRowsAndNumCols();
        
        // Calculate visual constants
        float totalHorizontalPadding = (numCols + 1) * padding;
        float totalVerticalPadding = (numRows + 1) * padding;
        
        // Calculate square size
        float squareWidth = (width - totalHorizontalPadding) / numCols;
        float squareHeight = (height - totalVerticalPadding) / numRows;
        squareSize = std::min (squareWidth, squareHeight); // so that they're still squares
        
        // Calculate offsets to center the grid
        xOffset = (width - (numCols * squareSize + totalHorizontalPadding)) / 2.0f;
        yOffset = (height - (numRows * squareSize + totalVerticalPadding)) / 2.0f;
    }
}
