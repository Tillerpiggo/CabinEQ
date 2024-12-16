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
    if (! event.mods.isShiftDown() || dataSource == nullptr)
    {
        hoveringId = -1;
        return;
    }
    
    auto hoveringRowAndCol = rowAndColFromMouseEvent (event);
    if (! hoveringRowAndCol.has_value())
    {
        hoveringId = -1;
        return;
    }
    
    // Figure out which sequence, if any, we're hovering over
    bool isHovering = false;
    NoiseSequenceGrid noiseGrid = dataSource->getNoiseGrid();
    
    for (const auto& sequence : noiseGrid.getNoiseSequences())
    {
        if (sequence.hasOriginAt (hoveringRowAndCol.value()))
        {
            isHovering = true;
            hoveringId = sequence.getId();
        }
    }
        
    if (! isHovering)
        hoveringId = -1;
}

void NoiseGridView::mouseDown (const juce::MouseEvent &event)
{
    auto mouseDownCoords = rowAndColFromMouseEvent (event);
    if (mouseDownCoords.has_value() && listener != nullptr && dataSource != nullptr)
    {
        if (event.mods.isRightButtonDown())
        {
            listener->removeSequence (mouseDownCoords.value());
            return;
        }
        
        if (isSquareAvailable (mouseDownCoords.value()))
        {
            addingCoords.push_back (mouseDownCoords.value());
        }
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
    
    if (isPositionNew && abs (mouseRowAndCol.first - prevRowAndCol.first) + abs (mouseRowAndCol.second - prevRowAndCol.second) == 1 && isSquareAvailable (mouseRowAndCol))
    {
        addingCoords.push_back (mouseRowAndCol);
    }
}

void NoiseGridView::mouseUp (const juce::MouseEvent &event)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    if (addingCoords.size() > 0)
    {
        int numSequences = (int) dataSource->getNoiseGrid().getNoiseSequences().size();
        listener->addSequenceWithCoords (addingCoords);
    }
    else
    {
        auto selectedRowAndCol = rowAndColFromMouseEvent (event);
        if (selectedRowAndCol.has_value())
            listener->toggleCoords (selectedRowAndCol.value());
    }
    
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
        
        updateVisualConstants();
        
        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numCols; col++)
            {
                // draw a square centered at the row and col
                if (isSquareAvailable ({ row, col }))
                    drawSquareAt (g, row, col, juce::Colours::white);
            }
        }
    }
}

void NoiseGridView::drawSquareAt (juce::Graphics& g, int row, int col, juce::Colour colour, float sizePercent)
{
    auto [x, y] = squareCoordsFromRowAndCol (row, col);
    juce::Rectangle<float> square (x + 0.5f * squareSize * (1.0f - sizePercent), y + 0.5f * squareSize * (1.0f - sizePercent), squareSize * sizePercent, squareSize * sizePercent);
    g.setColour (colour);
    g.fillRect (square);
}

void NoiseGridView::drawSequences (juce::Graphics& g)
{
    // Draw the adding sequence...
    if (dataSource != nullptr)
    {
        auto noiseGrid = dataSource->getNoiseGrid();
        auto noiseSequences = noiseGrid.getNoiseSequences();
        int nextId = noiseGrid.getNextAvailableId();
        drawSequence (g, addingCoords, sequenceColours[nextId % sequenceColours.size()]);
        for (int i = 0; i < noiseSequences.size(); ++i)
        {
            auto colour = sequenceColours[noiseSequences[i].getId() % sequenceColours.size()];
            if (! noiseSequences[i].isEnabled()) colour = juce::Colours::grey;
            drawSequence (g, noiseSequences[i].getAbsoluteCoords(), colour, hoveringId == noiseSequences[i].getId() ? 0.9f : 1.0f);
        }
    }
}

void NoiseGridView::drawSequence (juce::Graphics& g, std::vector<std::pair<int, int>> sequenceCoords, juce::Colour colour, float sizePercent)
{
    if (sequenceCoords.size() > 0)
    {
        juce::Path path;
        path.startNewSubPath (centerSquarePointFromCoords (sequenceCoords[0]));
        for (int i = 1; i < sequenceCoords.size(); ++i)
        {
            auto currCoords = sequenceCoords[i];
            drawSquareAt (g, currCoords.first, currCoords.second, colour.withLightness (0.8f), sizePercent);
            path.lineTo (centerSquarePointFromCoords (sequenceCoords[i]));
        }
        
        // Draw in the origin square on top of everything
        auto firstCoords = sequenceCoords[0];
        drawSquareAt (g, firstCoords.first, firstCoords.second, colour, sizePercent);
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

bool NoiseGridView::isSquareAvailable (std::pair<int, int> squareCoords)
{
    // First, make sure that the square isn't already taken by something else
    bool isSquareAvailable = true;
    auto noiseSequences = dataSource->getNoiseGrid().getNoiseSequences();
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        for (const auto& coords : noiseSequences[i].getAbsoluteCoords())
        {
            if (squareCoords.first == coords.first && squareCoords.second == coords.second)
            {
                isSquareAvailable = false;
            }
        }
    }
    
    return isSquareAvailable;
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
