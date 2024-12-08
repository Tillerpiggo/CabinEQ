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
    addSliderAndLabel (&speedSlider, &speedLabel, "Speed", 0.1f, 5.0f, 1.0f);
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
        {
            listener->setBandwidth (bandwidthSlider.getValue());
            animatedGlyph.setStrokeWidthFactor (bandwidthSlider.getValue());
        }
    });
    
    // Button actions
    addButtonAction (&playButton, [this](juce::Button*) {
        isPlaying = ! isPlaying;
        if (listener != nullptr)
            listener->setIsPlaying (isPlaying);
        
        playButton.setButtonText (isPlaying ? "Pause" : "Play");
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
    animatedGlyph.setListener (this);
    animatedGlyph.setDataSource (this);
}

GlyphView::~GlyphView()
{
}

void GlyphView::paint (juce::Graphics& g)
{
    // Do nothing, for now... (TODO: add in symbol)
    g.fillAll (CONTROL_BAR_BACKGROUND_COLOR);
}

void GlyphView::resized()
{
    float sidebarWidth = 300.0f;
    auto paddedBounds = getBounds().withX (0).withY (0);
    
    // Glyph side
    Layout glyphLayout (paddedBounds.withTrimmedRight (sidebarWidth), 8.0f);
    glyphLayout.addRow ({ Space (&animatedGlyph) });
    glyphLayout.updateComponentBounds();
    
    auto settingsBounds = paddedBounds.withTrimmedLeft (paddedBounds.getWidth() - sidebarWidth);
    Layout settingsLayout (settingsBounds, 8.0f);
    settingsLayout.addRow ({ Space (80), Space (&speedSlider) });
    settingsLayout.addRow ({ Space (80), Space (&bandwidthSlider) });
    settingsLayout.addRow ({ Space (&prevButton), Space (&nextButton) }, 30);
    settingsLayout.addRow ({ Space (&playButton) });
    settingsLayout.updateComponentBounds();
}

void GlyphView::setListener (Listener* listener)
{
    this->listener = listener;
    animatedGlyph.setListener (this);
}

void GlyphView::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
    animatedGlyph.setGlyph (dataSource->getCurrGlyph());
    updatePrevNextButtons();
}

void GlyphView::setSizeFactor (float sizeFactor)
{
    if (listener == nullptr)
        return;
    listener->setSizeFactor (sizeFactor);
}

void GlyphView::setCenterPos (juce::Point<float> centerPos)
{
    if (listener == nullptr)
        return;
    listener->setCenterPos (centerPos);
}

float GlyphView::getSizeFactor()
{
    if (dataSource == nullptr)
        return 1.0f; // default
    return dataSource->getSizeFactor();
}

juce::Point<float> GlyphView::getCenterPos()
{
    if (dataSource == nullptr)
        return { 0.0f, 0.0f }; // default
    return dataSource->getCenterPos();
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
