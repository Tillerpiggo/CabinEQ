/*
  ==============================================================================

    ArchetypeBar.h
    Created: 22 Dec 2024 8:55:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Listeners.h"
#include "Layout.h"
#include "ArchetypeView.h"
#include "Glyph.h"

// This displays a scrollable list of glyph archetypes that you can drag and drop from (and maybe add to in the future). Should be displayed in a viewport to be scrollable.
class ArchetypeBar  : public juce::Component
{
public:
    ArchetypeBar();
    ~ArchetypeBar() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setDataSource (GlyphViewDataSource* dataSource);
    
private:
    GlyphViewDataSource* dataSource = nullptr;
    std::vector<std::unique_ptr<ArchetypeView>> archetypeViews;
    
    juce::Colour BACKGROUND_COLOUR = juce::Colours::teal;
};