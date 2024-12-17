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
    
    sizeLabel.setText ("Resolution", juce::NotificationType::dontSendNotification);
    sizeLabel.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::bold)));
    sizeLabel.setJustificationType (juce::Justification::horizontallyCentred);
    
    addButton (&increaseSizeButton);
    addButton (&decreaseSizeButton);
    addAndMakeVisible (sizeLabel);
    
    addButtonAction (&increaseSizeButton, [this](juce::Button*) {
        if (listener != nullptr && dataSource != nullptr)
        {
            listener->scaleUpGrid();
            updateVisualConstants();
        }
    });
    addButtonAction (&decreaseSizeButton, [this](juce::Button*) {
        if (listener != nullptr && dataSource != nullptr)
        {
            listener->scaleDownGrid();
            updateVisualConstants();
        }
    });
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
    Layout layout (getBounds().withX (0).withY (0).withTrimmedLeft (getWidth() * 2.0f / 3.0f), 4);
    layout.addRow ({ Space (&increaseSizeButton), Space (&sizeLabel, 200), Space (&decreaseSizeButton) });
    layout.updateComponentBounds();
}

void NoiseGridView::mouseMove (const juce::MouseEvent &event)
{
    updateHovering (event);
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
            addingSequence = NoiseSequence (mouseDownCoords.value(), dataSource->getNoiseGrid().getNextAvailableId());
//            addingCoords.push_back (mouseDownCoords.value());
        }
        else
        {
            // Get the id, if any, of the selected sequence
            draggingId = dataSource->getSequenceIdAtCoords (mouseDownCoords.value());
            if (draggingId != -1)
                draggingSequence = dataSource->getNoiseGrid().getNoiseSequences()[draggingId];
        }
    }
}

void NoiseGridView::mouseDrag (const juce::MouseEvent &event)
{
    updateHovering (event);
    
    auto mouseDragCoords = rowAndColFromMouseEvent (event);
    
    // If we're dragging, handle it and return
    if (draggingId != -1)
    {
        std::cout << "dragging with non-0 draggingId" << std::endl;
        // Update the destination location
        if (! mouseDragCoords.has_value())
        {
            isPotentialDragLocationAvailable = false;
        }
        else
        {
            draggingSequence->moveOriginTo (mouseDragCoords.value());
            isPotentialDragLocationAvailable = canDragToPosition (mouseDragCoords.value());
        }
        return;
    }
    
    // Make sure we're dragging in a legit spot
    if (! mouseDragCoords.has_value() || ! addingSequence.has_value())
        return;
    
    auto addingCoords = addingSequence->getCoords();
    auto mouseRowAndCol = mouseDragCoords.value();
    auto prevRowAndCol = addingCoords[addingCoords.size() - 1];
    
    // If we backtrack, remove the last square
    if (addingCoords.size() > 1)
    {
        auto prevPrevRowAndCol = addingCoords[addingCoords.size() - 2];
        if (mouseRowAndCol.first == prevPrevRowAndCol.first && mouseRowAndCol.second == prevPrevRowAndCol.second)
        {
            addingSequence->removeLastCoords();
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
        addingSequence->addCoords (mouseRowAndCol);
    }
}

void NoiseGridView::mouseUp (const juce::MouseEvent &event)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    auto mouseUpRowAndCol = rowAndColFromMouseEvent (event);
    if (addingSequence.has_value())
    {
        listener->addSequence (addingSequence.value());
    }
    else if (mouseUpRowAndCol.has_value() && draggingId == -1)
    {
        listener->toggleCoords (mouseUpRowAndCol.value());
    }
    
    // Move the sequence if draggable
    if (isPotentialDragLocationAvailable && mouseUpRowAndCol.has_value())
    {
        listener->moveSequence (draggingId, mouseUpRowAndCol.value());
    }
    
    addingSequence.reset();
    draggingId = -1;
    isPotentialDragLocationAvailable = false;
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
        if (addingSequence.has_value())
            drawSequence (g, addingSequence.value(), colourForId (nextId));
        for (int i = 0; i < noiseSequences.size(); ++i)
        {
            auto colour = colourForId (noiseSequences[i].getId());
            if (! noiseSequences[i].getIsEnabled()) colour = juce::Colours::grey;
            drawSequence (g, noiseSequences[i], colour, 1.0f, hoveringId == noiseSequences[i].getId() ? 0.9f : 1.0f);
        }
        
        if (draggingId != -1 && isPotentialDragLocationAvailable)
        {
            drawSequence (g, draggingSequence.value(), colourForId (draggingId), 0.5f, 0.9f);
        }
    }
}

void NoiseGridView::drawSequence (juce::Graphics& g, NoiseSequence sequence, juce::Colour colour, float alpha, float sizePercent)
{
    auto sequenceCoords = sequence.getCoords();
    if (sequenceCoords.size() > 0)
    {
        juce::Path path;
        path.startNewSubPath (centerSquarePointFromCoords (sequenceCoords[0]));
        for (int i = 1; i < sequenceCoords.size(); ++i)
        {
            auto currCoords = sequenceCoords[i];
            drawSquareAt (g, currCoords.first, currCoords.second, juce::Colours::black, 1.0f); // to block out the white square
            drawSquareAt (g, currCoords.first, currCoords.second, colour.withLightness (0.8f).withAlpha ((sequence.getHits()[i] ? 1.0f : 0.3f) * alpha), sizePercent);
            drawSquareAt (g, currCoords.first, currCoords.second, juce::Colours::black, sizePercent * 0.9f);
            path.lineTo (centerSquarePointFromCoords (sequenceCoords[i]));
        }
        
        // Draw in the origin square on top of everything
        auto firstCoords = sequenceCoords[0];
        drawSquareAt (g, firstCoords.first, firstCoords.second, juce::Colours::black, 1.0f); // to block out the white square
        drawSquareAt (g, firstCoords.first, firstCoords.second, colour.withAlpha ((sequence.getHits()[0] ? 1.0f : 0.3f) * alpha), sizePercent);
    }
}

void NoiseGridView::updateHovering (const juce::MouseEvent& event)
{
    if (! event.mods.isShiftDown() || dataSource == nullptr || draggingId != -1)
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

bool NoiseGridView::canDragToPosition (std::pair<int, int> pos)
{
    if (draggingId == -1 || dataSource == nullptr)
        return false;
    
    // Check if any square in the dragging sequence is unavailable
    auto draggingSequenceCoords = draggingSequence->getCoords();
    for (const auto& draggingCoords : draggingSequenceCoords)
    {
        if (! isSquareAvailable (draggingCoords))
        {
            return false;
        }
    }
    
    return true;
}

bool NoiseGridView::isSquareAvailable (std::pair<int, int> squareCoords)
{
    // First, make sure that the square isn't already taken by something else
    bool isSquareAvailable = true;
    auto noiseSequences = dataSource->getNoiseGrid().getNoiseSequences();
    for (int i = 0; i < noiseSequences.size(); ++i)
    {
        for (const auto& coords : noiseSequences[i].getCoords())
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
            return std::pair<int, int> { row, col };
        }
    }

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


juce::Colour NoiseGridView::colourForId (int id)
{
    return sequenceColours[id % sequenceColours.size()];
}
