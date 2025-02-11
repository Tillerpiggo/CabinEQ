/*
  ==============================================================================

    AudioPlayerComponent.h
    Created: 10 Feb 2025 11:55:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Layout.h"
#include "BuildableComponent.h"
#include "Listeners.h"

enum AudioState
{
    Stopped,
    Starting,
    Playing,
    Stopping
};

// This is a component that allows one to play audio files they upload from their compueter
class AudioPlayerComponent  : public BuildableComponent
{
public:
    AudioPlayerComponent();
    ~AudioPlayerComponent();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (AudioPlayerComponentListener* listener);
    
private:
    juce::TextButton uploadFileButton { "Upload File..." };
    juce::TextButton playButton { "Play" };
    
    AudioState state;
    juce::AudioFormatManager audioFormatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> audioReaderSource;
    juce::AudioTransportSource audioTransportSource;
};
