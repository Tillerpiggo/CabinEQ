/*
  ==============================================================================

    ElevationCalibrationView.cpp
    Created: 23 Feb 2025 11:26:04am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ElevationCalibrationView.h"
#include "UIConstants.h"

ElevationCalibrationView::ElevationCalibrationView()
{
    addAndMakeVisible(plusButton);
    addAndMakeVisible(minusButton);
    
    plusButton.setButtonText("+");
    minusButton.setButtonText("-");
    
    plusButton.onClick = [this] {
        if (listener != nullptr && dataSource != nullptr && dataSource->canIncreaseNumRows())
            listener->increaseNumRows();
        updatePlusMinusButtons();
    };
    
    minusButton.onClick = [this] {
        if (listener != nullptr && dataSource != nullptr && dataSource->canDecreaseNumRows())
            listener->decreaseNumRows();
        updatePlusMinusButtons();
    };
    
    startTimerHz(120); // For animation updates
}

ElevationCalibrationView::~ElevationCalibrationView()
{
    stopTimer();
}

void ElevationCalibrationView::paint(juce::Graphics& g)
{
    g.fillAll(BACKGROUND_COLOUR);
    
    drawGridLines(g);
    drawRows(g);
}

void ElevationCalibrationView::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = 30;
    auto buttonWidth = 30;
    auto spacing = 5;
    
    plusButton.setBounds(bounds.getRight() - buttonWidth - spacing, 
                        bounds.getY() + spacing,
                        buttonWidth, 
                        buttonHeight);
                        
    minusButton.setBounds(plusButton.getX() - buttonWidth - spacing,
                         plusButton.getY(),
                         buttonWidth,
                         buttonHeight);
}

void ElevationCalibrationView::mouseMove(const juce::MouseEvent& event)
{
    repaint();
}

void ElevationCalibrationView::mouseDown(const juce::MouseEvent& event)
{
    
}

void ElevationCalibrationView::mouseDrag(const juce::MouseEvent& event)
{
    // Handle any drag behavior if needed
}

void ElevationCalibrationView::mouseUp(const juce::MouseEvent& event)
{
    auto rowIdx = rowIdxForMouseEvent(event);
    if (rowIdx.has_value() && listener != nullptr)
    {
        listener->selectedRowChanged(rowIdx.value());
    }
}

void ElevationCalibrationView::mouseExit(const juce::MouseEvent& event)
{
    repaint();
}

void ElevationCalibrationView::setListener(ElevationCalibrationListener* listener)
{
    this->listener = listener;
}

void ElevationCalibrationView::setDataSource(ElevationCalibrationDataSource* dataSource)
{
    this->dataSource = dataSource;
}

void ElevationCalibrationView::timerCallback()
{
    // Update animation state and repaint
    repaint();
}

void ElevationCalibrationView::updatePlusMinusButtons()
{
    if (dataSource == nullptr)
        return;
        
    plusButton.setEnabled(dataSource->canIncreaseNumRows());
    minusButton.setEnabled(dataSource->canDecreaseNumRows());
}

void ElevationCalibrationView::drawGridLines(juce::Graphics& g)
{
    // Draw horizontal grid lines
    auto bounds = getLocalBounds().toFloat();
    g.setColour(GRIDLINE_COLOUR);
    
    // Implementation will depend on your specific grid needs
}

void ElevationCalibrationView::drawRows(juce::Graphics& g)
{
    if (dataSource == nullptr)
        return;
        
    int numRows = dataSource->getNumRows();
    for (int i = 0; i < numRows; ++i)
    {
        drawRow(i, g);
    }
}

void ElevationCalibrationView::drawRow(int rowIdx, juce::Graphics& g)
{
    if (dataSource == nullptr)
        return;
        
    auto rowBounds = getRowRect(rowIdx);
    bool isSelected = (rowIdx == dataSource->getSelectedRow());
    
    g.setColour(isSelected ? ON_COLOUR : ROW_COLOUR);
    g.fillRect(rowBounds);
}

juce::Rectangle<float> ElevationCalibrationView::getRowRect(int rowIdx)
{
    if (dataSource == nullptr)
        return {};
        
    auto bounds = getLocalBounds().toFloat();
    int numRows = dataSource->getNumRows();
    float rowHeight = bounds.getHeight() / numRows;
    
    return bounds.withHeight(rowHeight).withY(bounds.getY() + rowIdx * rowHeight);
}

std::optional<int> ElevationCalibrationView::rowIdxForMouseEvent(const juce::MouseEvent& event)
{
    if (dataSource == nullptr)
        return std::nullopt;
        
    auto bounds = getLocalBounds().toFloat();
    int numRows = dataSource->getNumRows();
    float rowHeight = bounds.getHeight() / numRows;
    
    int rowIdx = (event.y - bounds.getY()) / rowHeight;
    if (rowIdx >= 0 && rowIdx < numRows)
        return rowIdx;
        
    return std::nullopt;
}
