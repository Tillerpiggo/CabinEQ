/*
  ==============================================================================

    AnimatedGlyph.cpp
    Created: 14 Nov 2024 4:21:43pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AnimatedGlyph.h"

AnimatedGlyph::AnimatedGlyph()
{}

void AnimatedGlyph::setGlyph (Glyph glyph)
{
    this->glyph = glyph;
}

void AnimatedGlyph::setStrokeWidth (float strokeWidth)
{
    this->strokeWidth = strokeWidth;
}

void AnimatedGlyph::paint (juce::Graphics& g)
{
    // TODO: draw the glyph
}

void AnimatedGlyph::resized()
{
    // Do nothing for now...
}

void AnimatedGlyph::setDataSource (DataSource* dataSource)
{
    this->dataSource = dataSource;
}

void AnimatedGlyph::timerCallback()
{
    repaint();
}
