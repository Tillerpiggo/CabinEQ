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
    
}

void NoiseGridView::mouseDrag (const juce::MouseEvent &event)
{
    
}

void NoiseGridView::mouseUp (const juce::MouseEvent &event)
{
    
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
}

void NoiseGridView::setDataSource (NoiseGridViewDataSource* dataSource)
{
    this->dataSource = dataSource;
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
    float width = getWidth();
    float height = getHeight();
    float padding = 10.0f;
    
    if (true || listener != nullptr && dataSource != nullptr)
    {
//        auto [numRows, numCols] = dataSource->getNumRowsAndNumCols();
        int numRows = 3;
        int numCols = 3;
        
        // Calculate visual constants
        float totalHorizontalPadding = (numCols + 1) * padding;
        float totalVerticalPadding = (numRows + 1) * padding;
        
        // Calculate square size
        float squareWidth = (width - totalHorizontalPadding) / numCols;
        float squareHeight = (height - totalVerticalPadding) / numRows;
        float squareSize = std::min (squareWidth, squareHeight); // so that they're still squares
        
        // Calculate offsets to center the grid
        float xOffset = (width - (numCols * squareSize + totalHorizontalPadding)) / 2.0f;
        float yOffset = (height - (numRows * squareSize + totalVerticalPadding)) / 2.0f;
        
        for (int row = 0; row < numRows; row++)
        {
            for (int col = 0; col < numCols; col++)
            {
                // draw a square centered at the right position
                float x = xOffset + padding + col * (squareSize + padding);
                float y = yOffset + padding + row * (squareSize + padding);
                
                // actually draw the square
                juce::Rectangle<float> square (x, y, squareSize, squareSize);
                g.setColour (juce::Colours::white);
                g.fillRect (square);
            }
        }
    }
}

void NoiseGridView::drawSequences (juce::Graphics& g)
{
    // TODO
}
