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

void CheckerboardView::updateCheckerboardChanged()
{
    if (dataSource != nullptr)
    {
        this->checkerboard = dataSource->getCheckerboard();
        repaint();
    }
}

void CheckerboardView::drawGridLines (juce::Graphics& g)
{
    // Draw border
    g.setColour (BORDER_COLOUR);
    g.drawRect (0, 0, getWidth(), getHeight());
    
    // Draw grid lines
    g.setColour (GRIDLINE_COLOUR);
    int resolution = checkerboard.getResolution();
    int numHorizontalLines = resolution;
    int numVerticalLines = resolution;
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
    int resolution = checkerboard.getResolution();
    for (int freqIdx = 0; freqIdx < resolution; ++freqIdx)
    {
        for (int panIdx = 0; panIdx < resolution; ++panIdx)
        {
            if (grid[panIdx][freqIdx])
                drawSquare (freqIdx, panIdx, isPlaying ? SQUARE_ON_COLOUR : SQUARE_OFF_COLOUR, g);
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
    int resolution = checkerboard.getResolution();
    if (freqIdx < 0 || freqIdx >= resolution)
    {
        std::cerr << "ERR: getting rect for freq idx and pan idx with freqIdx out of bounds (freqIdx: " << freqIdx << ", resolution: " << resolution << std::endl;
        return { 0, 0, 0, 0 };
    }
    
    if (panIdx < 0 || panIdx >= resolution)
    {
        std::cerr << "ERR: getting rect for freq idx and pan idx with panIdx out of bounds (panIdx: " << panIdx << ", resolution: " << resolution << std::endl;
        return { 0, 0, 0, 0 };
    }
    
    // Get width and height
    float width = (float) getWidth() / (float) resolution;
    float height = (float) getHeight() / (float) resolution;
    
    // Get x and y coords
    
    float normalizedX = (float) panIdx / (float) resolution;
    float normalizedY = (float) (freqIdx + 1) / (float) resolution; // +1 so that when we flip it upside, y starts at the top instead of the bottom
    float absX = normalizedX * (float) getWidth();
    float absY = (1.0f - normalizedY) * (float) getHeight();
    
    return { absX, absY, width, height };
}


