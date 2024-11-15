/*
  ==============================================================================

    GlyphView.cpp
    Created: 14 Nov 2024 3:40:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GlyphView.h"

GlyphView::GlyphView()
{
    addSliderAndLabel (&speedSlider, &speedLabel, "Speed", 0.5f, 4.0f, 1.0f);
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.1f, 4.0f, 1.0f);
    
    addButton (&playButton);
    addButton (&prevButton);
    addButton (&nextButton);
    
    // Slider actions
    addSliderAction (&speedSlider, [this](juce::Slider*) {
        if (listener != nullptr)
            listener->setSpeed (speedSlider.getValue());
    });
    addSliderAction (&bandwidthSlider, [this](juce::Slider*) {
        if (listener != nullptr)
            listener->setBandwidth (bandwidthSlider.getValue());
    });
    
    // Button actions
    addButtonAction (&playButton, [this](juce::Button*) {
        isPlaying = ! isPlaying;
        if (listener != nullptr)
            listener->setIsPlaying (isPlaying);
    });
    addButtonAction (&prevButton, [this](juce::Button*) {
        if (listener != nullptr && dataSource != nullptr)
        {
            listener->goToPrev();
            glyph = dataSource->getCurrGlyph();
            updatePrevNextButtons();
        }
    });
    addButtonAction (&nextButton, [this](juce::Button*) {
        if (listener != nullptr && dataSource != nullptr)
        {
            listener->goToNext();
            glyph = dataSource->getCurrGlyph();
            updatePrevNextButtons();
        }
    });
}

GlyphView::~GlyphView()
{
}

void GlyphView::paint (juce::Graphics& g)
{
    // Do nothing, for now... (TODO: add in symbol)
}

void GlyphView::resized()
{
    Layout layout (getBounds(), 8);
//    layout.addRow ({ Space (80), Space (&animatedGlyph), Space (80) }, 120);
    layout.addRow ({ Space (80), Space (&speedSlider), Space (80), Space (&bandwidthSlider) });
    layout.addRow ({ Space (80), Space (&prevButton), Space (&playButton), Space (&nextButton), Space (80) });
    layout.updateComponentBounds();
}

void GlyphView::setListener (Listener* listener)
{
    this->listener = listener;
}

void GlyphView::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
}

void GlyphView::updatePrevNextButtons()
{
    if (dataSource != nullptr)
    {
        prevButton.setEnabled (dataSource->hasPrev());
        nextButton.setEnabled (dataSource->hasNext());
    }
}
