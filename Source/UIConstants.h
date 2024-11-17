/*
  ==============================================================================

    UIConstants.h
    Created: 16 Nov 2024 11:18:59pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This fie contains UI constants like colors, spacing, and more

// CabinPeqGraph
constexpr float DOT_SIZE_SELECTED = 5.5f;
constexpr float DOT_SIZE_DRAGGING = 8.0f;
constexpr float DOT_SIZE_DEFAULT = 3.5f;
constexpr float DOT_PADDING = 3.0f;
constexpr float CURVE_THICKNESS = 2.5f;
const juce::ColourGradient BACKGROUND_GRADIENT (
                                                                             juce::Colours::black.withAlpha (0.95f), 0.0f, static_cast<float> (400), // Bottom
                                                                             juce::Colours::black.withAlpha (0.6f), 0.0f, 0.0f, // Top edge
                                                                             false);
// GlyphView
const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colours::black.withAlpha (0.7f);
