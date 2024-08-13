/*
  ==============================================================================

    CabinEQPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNode.h"
#include "CabinEQGraph.h"

class CabinEQPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public CabinEQGraph::Listener,
                      public StartupMVPAudioProcessor::Listener
{
public:
    CabinEQPage (StartupMVPAudioProcessor& p);
    ~CabinEQPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // CabinEQGraphListener methods
    int addNode (float freq, float ampl) override;
    void updateNode (int id, float freq, float ampl) override;
    void removeNode (int id) override;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
    void testValueAt (float freq) override;
    void stopPlaying() override;
    void stopTesting() override;
    float getCurrPlayingFreq() override;
    float getCurrTestingFreq() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    
    void didLoadData() override;
    
private:
    StartupMVPAudioProcessor& processor;
    juce::String profileId;
    
    CabinEQGraph cabinEQGraph;
    juce::ComboBox dropdownProfiles;
    juce::Slider referenceSlider;
    juce::AlertWindow alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
};
