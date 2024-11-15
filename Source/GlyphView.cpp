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
            listener->goToPrevGlyph();
            animatedGlyph.setGlyph (dataSource->getCurrGlyph());
            updatePrevNextButtons();
        }
    });
    addButtonAction (&nextButton, [this](juce::Button*) {
        if (listener != nullptr && dataSource != nullptr)
        {
            listener->goToNextGlyph();
            animatedGlyph.setGlyph (dataSource->getCurrGlyph());
            updatePrevNextButtons();
        }
    });
    
    addAndMakeVisible (animatedGlyph);
    animatedGlyph.setDataSource (this);
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
    float glyphWidth = getBounds().getWidth() / 2.0f;
    
    // Glyph side
    Layout glyphLayout (getBounds().withTrimmedRight (getBounds().getWidth() - glyphWidth), 8);
    glyphLayout.addRow ({ Space (&animatedGlyph) });
    glyphLayout.addRow ({ Space (&prevButton), Space (&nextButton) }, 30);
    glyphLayout.updateComponentBounds();
    
    Layout settingsLayout (getBounds().withTrimmedLeft (glyphWidth), 8);
    settingsLayout.addRow ({ Space (80), Space (&speedSlider), Space (80), Space (&bandwidthSlider) });
    settingsLayout.addRow ({ Space (80), Space (&playButton), Space (80) });
    settingsLayout.updateComponentBounds();
}

void GlyphView::setListener (Listener* listener)
{
    this->listener = listener;
}

void GlyphView::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
    animatedGlyph.setGlyph (dataSource->getCurrGlyph());
    updatePrevNextButtons();
}

float GlyphView::getCurrTime()
{
    if (dataSource == nullptr)
        return 0.0f;
    return dataSource->getCurrPlayingTime();
}

void GlyphView::updatePrevNextButtons()
{
    if (dataSource != nullptr)
    {
        prevButton.setEnabled (dataSource->hasPrevGlyph());
        nextButton.setEnabled (dataSource->hasNextGlyph());
    }
}
