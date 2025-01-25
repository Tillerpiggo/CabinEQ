/*
  ==============================================================================

    CalibrationView.h
    Created: 22 Dec 2024 8:38:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"
#include "Listeners.h"
#include "ArchetypeBar.h"
#include "GlyphGridView.h"

// This provides a UI for glyph calibration. It includes  a view that lets you drag and move around glyphs, a view that lets you add glyphs from a list, and a view with settings that impact playback.
class CalibrationView  : public BuildableComponent,
                         public juce::ComboBox::Listener
{
public:
    CalibrationView();
    ~CalibrationView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (GlyphViewListener* listener);
    void setCalibrationListener (CalibrationListener* calibrationListener);
    void setDataSource (GlyphViewDataSource* dataSource);
    
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
private:
    CalibrationListener* calibrationListener = nullptr;
    
    // Glyph View
    GlyphGridView glyphGridView;
    
    // Archetype Bar
    juce::Viewport archetypeViewport;
    ArchetypeBar archetypeBar;
    
    // Calibration settings
    juce::Slider speedSlider;
    juce::Label speedLabel;
    juce::Slider bandwidthSlider;
    juce::Label bandwidthLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton iirButton { "IIR" };
    juce::TextButton updateFilterButton { "Update" };
    juce::ComboBox qualityComboBox;
    juce::ComboBox scalingComboBox;
    juce::ComboBox erbComboBox;
    bool isPlaying = false;
    bool isIIR = true;
    bool isFIRFilterUpdated = false;
};

