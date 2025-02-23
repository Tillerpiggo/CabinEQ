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

// SPACING ========================================================

// Globals
constexpr float TOP_ROW_HEIGHT = 40;
constexpr float PROFILE_VIEW_WIDTH = 100;

// Profile View
constexpr float PROFILE_ROW_HEIGHT = 30;
constexpr float PROFILE_ROW_SPACING = 10;

// Graph
constexpr int NUM_POINTS = 200;
constexpr float DOT_SIZE_SELECTED = 5.5f;
constexpr float DOT_SIZE_DRAGGING = 8.0f;
constexpr float DOT_SIZE_DEFAULT = 3.5f;
constexpr float DOT_PADDING = 3.0f;
constexpr float CURVE_THICKNESS = 2.5f;

// Colors ==========================================================

// Globals
const juce::Colour ON_COLOUR = juce::Colours::purple;
const juce::Colour OFF_COLOUR = juce::Colours::darkgrey;
const juce::Colour BACKGROUND_COLOUR = juce::Colours::black;
const juce::Colour PRIMARY_TEXT_COLOUR = juce::Colours::white;
const juce::Colour SECONDARY_TEXT_COLOUR = juce::Colours::darkgrey;
const juce::Colour HOVER_COLOUR = juce::Colours::darkgrey;
const juce::Colour DIVIDER_COLOUR = juce::Colours::darkgrey;

// Profile View
const juce::Colour PROFILE_ROW_BACKGROUND_COLOUR = juce::Colours::darkgrey;
const juce::Colour PROFILE_ROW_TEXT_COLOUR = juce::Colours::white;

// Graph
const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour(0xFF31C1FF);  // Blue
const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour(0xFF1E88E5);  // Darker Blue
const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour(0xFFFFA726);  // Orange
const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour(0xFFEF5350);  // Red
const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour(0xFFE91E63);  // Pink

const juce::Colour GRAPH_BACKGROUND_COLOUR = juce::Colours::black;
const juce::Colour GRAPH_LINE_COLOUR = juce::Colours::darkgrey;
const juce::Colour VOLUME_DOT_COLOUR = juce::Colours::purple;
const juce::Colour VOLUME_RECT_COLOUR = juce::Colours::lightgrey;
const juce::Colour NUMBERS_COLOUR = juce::Colours::lightgrey;
const juce::Colour SELECTION_COLOUR = juce::Colours::white.withAlpha(0.3f);
const juce::Colour SELECTION_BORDER_COLOUR = juce::Colours::white;
const juce::Colour SELECTED_BAND_COLOUR = juce::Colours::white.withAlpha(0.8f);
const juce::Colour LINE_COLOUR = juce::Colours::darkgrey.withMultipliedLightness(0.5f);
const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colours::lightgrey;

// Fonts ===========================================================
const juce::Font ADD_PROFILE_FONT = juce::Font (juce::FontOptions (12.0f));
const juce::Font PROFILE_ROW_FONT = juce::Font (juce::FontOptions (12.0f));
const juce::Font GRAPH_NUMBERS_FONT = juce::Font (juce::FontOptions (10.0f));
const juce::Font BODY_FONT = juce::Font (juce::FontOptions (14.0f));

// // CabinEqPage
// const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colours::black.withAlpha (1.0f);
// const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colours::black.withAlpha (1.0f);

// // CabinPeqGraph
// constexpr int NUM_POINTS = 200;
// constexpr float DOT_SIZE_SELECTED = 5.5f;
// constexpr float DOT_SIZE_DRAGGING = 8.0f;
// constexpr float DOT_SIZE_DEFAULT = 3.5f;
// constexpr float DOT_PADDING = 3.0f;
// constexpr float CURVE_THICKNESS = 2.5f;
// const juce::ColourGradient BACKGROUND_GRADIENT (
//                                                                              juce::Colours::black.withAlpha (0.95f), 0.0f, static_cast<float> (400), // Bottom
//                                                                              juce::Colours::black.withAlpha (0.6f), 0.0f, 0.0f, // Top edge
//                                                                              false);
// const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colours::lightgrey;
// const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour(0xFF31C1FF);  // Blue
// const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour(0xFF1E88E5);  // Darker Blue
// const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour(0xFFFFA726);  // Orange
// const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour(0xFFEF5350);  // Red
// const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour(0xFFE91E63);  // Pink
// const juce::Colour SELECTION_COLOUR = juce::Colours::white.withAlpha(0.3f);
// const juce::Colour SELECTION_BORDER_COLOUR = juce::Colours::white;
// const juce::Colour SELECTED_BAND_COLOUR = juce::Colours::white.withAlpha(0.8f);
// const juce::Colour CENTER_LINE_COLOUR = juce::Colours::darkgrey.withMultipliedLightness(0.5f);
// const juce::Colour LINE_COLOUR = juce::Colours::darkgrey.withMultipliedLightness(0.5f);
// const juce::Colour VOLUME_RECT_COLOUR = juce::Colours::lightgrey.withAlpha(0.3f);

// // GlyphView
// const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colours::black.withAlpha (0.7f);

// LookAndFeel
const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(255, 215, 0); // Gold
const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(20, 20, 20); // Very dark gray, almost black
const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(70, 70, 70); // Dark gray
const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(30, 30, 30); // Very dark gray
const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(255, 105, 180); // Purple
const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 200, 200); // Light gray
const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 255, 255); // White
const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(100, 100, 100); // Medium gray

// // Graph Background
// const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB(0.1, 0.1, 0.2);
