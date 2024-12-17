/*
  ==============================================================================

    NoiseGridView.h
    Created: 14 Dec 2024 5:04:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "NoiseSequenceGrid.h"
#include "Listeners.h"
#include "Layout.h"

class NoiseGridView  : public BuildableComponent,
                       public juce::Timer
{
public:
    NoiseGridView();
    ~NoiseGridView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void timerCallback() override;
    
    void setListener (NoiseGridViewListener* listener);
    void setDataSource (NoiseGridViewDataSource* dataSource);
    void removeListener();
    void removeDataSource();
    
private:
    void drawSquares (juce::Graphics& g);
    void drawSequences (juce::Graphics& g);
    void drawSequence (juce::Graphics& g, NoiseSequence sequence, juce::Colour colour, float sizePercent = 1.0f);
    void drawSquareAt (juce::Graphics& g, int row, int col, juce::Colour colour, float sizePercent = 1.0f);
    juce::Point<float> squareCoordsFromRowAndCol (int row, int col);
    juce::Point<float> centerSquarePointFromCoords (std::pair<int, int> coords);
    bool isSquareAvailable (std::pair<int, int> squareCoords);
    std::optional<std::pair<int, int>> rowAndColFromMouseEvent (const juce::MouseEvent& event);
    void updateVisualConstants();
    
    juce::Rectangle<float> squareBoundsAtCoords (std::pair<int, int> coords);
    
    NoiseGridViewListener* listener = nullptr;
    NoiseGridViewDataSource* dataSource = nullptr;
    
    // Buttons
    juce::TextButton increaseSizeButton { "+" };
    juce::TextButton decreaseSizeButton { "-" };
    juce::Label sizeLabel;
    
    // Interaction variables
    std::optional<NoiseSequence> addingSequence;
    int hoveringId = -1;
    int draggingId = -1;
    
    // Visual constants
    float xOffset;
    float yOffset;
    float squareSize;
    float padding = 10.0f;
    std::vector<juce::Colour> sequenceColours { juce::Colours::blue, juce::Colours::orange, juce::Colours::green, juce::Colours::purple, juce::Colours::red };
};
