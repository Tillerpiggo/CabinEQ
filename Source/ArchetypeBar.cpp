/*
  ==============================================================================

    ArchetypeBar.cpp
    Created: 22 Dec 2024 8:55:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArchetypeBar.h"

ArchetypeBar::ArchetypeBar()
{
    
}

ArchetypeBar::~ArchetypeBar()
{
    // should release archetypeViews, probably, but they might release on their own anyways idk
}

void ArchetypeBar::paint (juce::Graphics& g)
{
    // Don't need to paint anything
    g.fillAll (BACKGROUND_COLOUR);
}

void ArchetypeBar::resized()
{
    // Calculate bounds to be at 0, 0, with the default width, and height proportional to # of archetypes
    juce::Rectangle<int> bounds (0, 0, getWidth(), getWidth() * (int) archetypeViews.size());
    
    Layout layout (getBounds().withX (0).withY (0), 8.0f);
    for (int i = 0; i < archetypeViews.size(); ++i)
    {
        layout.addRow ({ Space (archetypeViews[i].get()) });
    }
    layout.updateComponentBounds();
}

void ArchetypeBar::setDataSource (GlyphViewDataSource* dataSource)
{
    this->dataSource = dataSource;
    for (const auto& archetype : dataSource->getArchetypalGlyphs())
    {
        archetypeViews.push_back (std::make_unique<ArchetypeView> (archetype));
    }
    
    for (int i = 0; i < archetypeViews.size(); ++i)
        addAndMakeVisible (archetypeViews[i].get());
    
//    setSize (getWidth(), getWidth() * (int) archetypeViews.size());
}
