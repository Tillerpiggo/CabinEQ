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

// CabinEqPage
const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colours::black.withAlpha (1.0f);
const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colours::black.withAlpha (1.0f);

// CabinPeqGraph
const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.0f, 0.5f, 1.0f, 1.0f);
const juce::Colour CURVE_GRADIENT_COLOR_2 =  juce::Colour::fromFloatRGBA (0.0f, 0.75f, 1.0f, 1.0f);
const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 0.75f, 1.0f);
const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 0.3f, 1.0f);
const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.7f, 1.0f, 0.3f, 1.0f);

// GlyphView
const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colours::black.withAlpha (0.7f);

// LookAndFeel
const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(255, 215, 0); // Gold
const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(20, 20, 20); // Very dark gray, almost black
const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(70, 70, 70); // Dark gray
const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(30, 30, 30); // Very dark gray
const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(255, 105, 180); // Purple
const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 200, 200); // Light gray
const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 255, 255); // White
const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(100, 100, 100); // Medium gray

// AnimatedGlyph
const juce::Colour STROKE_COLOR = juce::Colours::lightblue;
const juce::Colour PLAYING_DOT_COLOR = juce::Colours::purple;
const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::lightgreen;

// Sunset
//// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(1.0f, 0.8f, 0.6f, 0.6f); // Light orange
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.6f, 0.2f, 0.0f, 0.95f); // Dark red-orange
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (1.0f, 0.6f, 0.0f, 1.0f); // Orange
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (1.0f, 0.4f, 0.0f, 1.0f); // Dark orange
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (1.0f, 0.2f, 0.0f, 1.0f); // Red-orange
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.8f, 0.0f, 0.0f, 1.0f); // Dark red
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.6f, 0.0f, 0.0f, 1.0f); // Very dark red
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.2f, 0.1f, 0.0f, 0.7f); // Very dark brown
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(255, 140, 0); // Dark orange
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(40, 20, 0); // Very dark brown
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(120, 60, 0); // Dark brown
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(60, 30, 0); // Very dark brown
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(255, 69, 0); // Red-orange
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 140, 100); // Light brown
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 220, 200); // Very light orange
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(140, 80, 40); // Saddle brown
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::darkorange;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::orangered;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::coral;

//// Minty Fresh
//// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.8f, 1.0f, 0.8f, 0.6f); // Light mint
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.0f, 0.4f, 0.2f, 0.95f); // Dark teal
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 0.6f, 1.0f); // Mint
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.0f, 0.8f, 0.6f, 1.0f); // Turquoise
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 0.6f, 0.6f, 1.0f); // Teal
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 0.4f, 0.4f, 1.0f); // Dark teal
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 0.2f, 1.0f); // Very dark teal
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 0.1f, 0.7f); // Very dark green
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(0, 255, 127); // Spring green
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(0, 40, 20); // Very dark green
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(0, 100, 50); // Dark green
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(0, 60, 30); // Very dark green
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(0, 255, 127); // Spring green
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 220, 200); // Light mint
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 255, 255); // White
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(0, 100, 80); // Teal
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::lightgreen;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::limegreen;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::springgreen;

//// Electric Purple
//// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.8f, 0.6f, 1.0f, 0.6f); // Light lavender
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.2f, 0.0f, 0.4f, 0.95f); // Dark indigo
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.6f, 0.0f, 1.0f, 1.0f); // Purple
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.6f, 0.0f, 0.8f, 1.0f); // Dark purple
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.6f, 0.0f, 0.6f, 1.0f); // Violet
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.4f, 0.0f, 0.4f, 1.0f); // Dark violet
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.2f, 0.0f, 0.2f, 1.0f); // Very dark violet
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.1f, 0.0f, 0.2f, 0.7f); // Very dark purple
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(191, 64, 191); // Medium purple
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(20, 0, 40); // Very dark purple
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(50, 0, 100); // Dark purple
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(30, 0, 60); // Very dark purple
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(191, 64, 191); // Medium purple
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 180, 220); // Light lavender
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 240, 255); // Very light lavender
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(80, 40, 120); // Dark violet
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::violet;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::orchid;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::magenta;

//// Space Odyssey
//// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.2f, 0.6f); // Very dark blue
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.1f, 0.95f); // Almost black
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.8f, 0.8f, 0.8f, 1.0f); // Light gray for better contrast
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 1.0f, 1.0f); // Cyan
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.0f, 0.8f, 1.0f, 1.0f); // Light blue
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 0.6f, 1.0f, 1.0f); // Medium blue
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 0.4f, 1.0f, 1.0f); // Blue
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 1.0f, 1.0f); // Dark blue
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.05f, 0.05f, 0.1f, 0.7f); // Very dark blue, slightly lighter than background
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(200, 200, 255); // Light blue-gray
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(10, 10, 30); // Very dark blue
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(40, 40, 80); // Dark blue
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(20, 20, 40); // Very dark blue
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(0, 255, 255); // Cyan
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(150, 150, 200); // Light blue-gray
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 255, 255); // White
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(60, 60, 100); // Medium blue
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::lightblue;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::cyan;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::skyblue;


// Space Odyssey (Updated)
// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.1f, 0.6f); // Extremely dark blue
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.0f, 0.0f, 0.05f, 0.95f); // Almost black
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.5f, 0.5f, 0.5f, 0.5f); // Faded gray for less emphasis
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 1.0f, 1.0f); // Cyan
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.0f, 0.8f, 1.0f, 1.0f); // Light blue
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 0.6f, 1.0f, 1.0f); // Medium blue
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 0.4f, 1.0f, 1.0f); // Blue
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 1.0f, 1.0f); // Dark blue
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.03f, 0.03f, 0.08f, 0.7f); // Extremely dark blue, slightly lighter than background
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(200, 200, 255); // Light blue-gray
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(5, 5, 15); // Extremely dark blue
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(30, 30, 60); // Very dark blue
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(15, 15, 30); // Extremely dark blue
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(0, 255, 255); // Cyan
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(150, 150, 200); // Light blue-gray
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 255, 255); // White
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(50, 50, 80); // Dark blue
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::lightblue;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::cyan;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::skyblue;

//// Galaxy
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.05f, 0.0f, 0.1f, 0.6f); // Very dark purple
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.02f, 0.0f, 0.05f, 0.95f); // Almost black with a hint of purple
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.6f, 0.6f, 0.6f, 0.4f); // Faded gray for less emphasis
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (1.0f, 0.0f, 1.0f, 1.0f); // Magenta
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.8f, 0.0f, 1.0f, 1.0f); // Purple
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.6f, 0.0f, 1.0f, 1.0f); // Deep purple
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.4f, 0.0f, 1.0f, 1.0f); // Indigo
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.2f, 0.0f, 1.0f, 1.0f); // Deep indigo
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.08f, 0.03f, 0.1f, 0.7f); // Very dark purple, slightly lighter than background
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(255, 200, 255); // Light magenta
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(20, 10, 30); // Very dark purple
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(60, 30, 80); // Dark purple
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(30, 15, 45); // Very dark purple
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(255, 0, 255); // Magenta
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(200, 150, 200); // Light purple
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(255, 220, 255); // Very light magenta
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(80, 50, 100); // Medium purple
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::violet;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::magenta;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::mediumorchid;

// Galaxy (updated)
// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.0f, 0.05f, 0.1f, 0.6f); // Very dark blue-green
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.0f, 0.02f, 0.05f, 0.95f); // Almost black with a hint of blue-green
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.4f, 0.4f, 0.4f, 0.4f); // Darker faded gray for less emphasis
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 0.0f, 1.0f); // Bright green
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.0f, 0.8f, 0.2f, 1.0f); // Lighter green
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 0.6f, 0.4f, 1.0f); // Teal
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 0.4f, 0.6f, 1.0f); // Light blue
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 0.8f, 1.0f); // Dark blue
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.03f, 0.08f, 0.1f, 0.7f); // Very dark blue-green, slightly lighter than background
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(200, 255, 200); // Light green
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(10, 20, 30); // Very dark blue-green
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(30, 60, 80); // Dark blue-green
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(15, 30, 45); // Very dark blue-green
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(0, 255, 0); // Bright green
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(150, 200, 200); // Light teal
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(220, 255, 220); // Very light green
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(50, 80, 100); // Medium blue-green
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::lightgreen;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::limegreen;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::mediumaquamarine;

//// Galaxy (flipped)
//// CabinEqPage
//const juce::Colour BACKGROUND_GRADIENT_LIGHT = juce::Colour::fromFloatRGBA(0.0f, 0.05f, 0.1f, 0.6f); // Very dark blue-green
//const juce::Colour BACKGROUND_GRADIENT_DARK = juce::Colour::fromFloatRGBA(0.0f, 0.02f, 0.05f, 0.95f); // Almost black with a hint of blue-green
//// CabinPeqGraph
//const juce::Colour CURVE_GRAYSCALE_COLOR = juce::Colour::fromFloatRGBA (0.4f, 0.4f, 0.4f, 0.4f); // Darker faded gray for less emphasis
//const juce::Colour CURVE_GRADIENT_COLOR_1 = juce::Colour::fromFloatRGBA(0.0f, 0.2f, 0.8f, 1.0f); // Dark blue
//const juce::Colour CURVE_GRADIENT_COLOR_2 = juce::Colour::fromFloatRGBA (0.0f, 0.4f, 0.6f, 1.0f); // Light blue
//const juce::Colour CURVE_GRADIENT_COLOR_3 = juce::Colour::fromFloatRGBA (0.0f, 0.6f, 0.4f, 1.0f); // Teal
//const juce::Colour CURVE_GRADIENT_COLOR_4 = juce::Colour::fromFloatRGBA (0.0f, 0.8f, 0.2f, 1.0f); // Lighter green
//const juce::Colour CURVE_GRADIENT_COLOR_5 = juce::Colour::fromFloatRGBA (0.0f, 1.0f, 0.0f, 1.0f); // Bright green
//// GlyphView
//const juce::Colour CONTROL_BAR_BACKGROUND_COLOR = juce::Colour::fromFloatRGBA(0.03f, 0.08f, 0.1f, 0.7f); // Very dark blue-green, slightly lighter than background
//// LookAndFeel
//const juce::Colour SLIDER_THUMB_COLOR = juce::Colour::fromRGB(200, 255, 200); // Light green
//const juce::Colour SLIDER_BACKGROUND_COLOR = juce::Colour::fromRGB(10, 20, 30); // Very dark blue-green
//const juce::Colour SLIDER_TRACK_COLOR = juce::Colour::fromRGB(30, 60, 80); // Dark blue-green
//const juce::Colour BUTTON_OFF_COLOR = juce::Colour::fromRGB(15, 30, 45); // Very dark blue-green
//const juce::Colour BUTTON_ON_COLOR = juce::Colour::fromRGB(0, 255, 0); // Bright green
//const juce::Colour BUTTON_OFF_TEXT_COLOR = juce::Colour::fromRGB(150, 200, 200); // Light teal
//const juce::Colour BUTTON_ON_TEXT_COLOR = juce::Colour::fromRGB(220, 255, 220); // Very light green
//const juce::Colour BUTTON_OUTLINE_COLOR = juce::Colour::fromRGB(50, 80, 100); // Medium blue-green
//// AnimatedGlyph
//const juce::Colour STROKE_COLOR = juce::Colours::lightgreen;
//const juce::Colour PLAYING_DOT_COLOR = juce::Colours::limegreen;
//const juce::Colour DRAGGING_DOT_COLOR = juce::Colours::mediumaquamarine;
