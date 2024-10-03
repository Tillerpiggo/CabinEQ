/*
  ==============================================================================

    ColorTheme.h
    Created: 3 Oct 2024 4:36:25pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ColorTheme
{
public:
    virtual ~ColorTheme() = default;

    // Primary Colors
    virtual juce::Colour getBackgroundBase() const = 0;
    virtual juce::Colour getTopBottomBars() const = 0;
    virtual juce::Colour getAccentColor() const = 0;
    virtual juce::Colour getSecondaryAccent() const = 0;
    virtual juce::Colour getTextLines() const = 0;

    // Top and Bottom Bars
    virtual juce::Colour getBarBackgroundColor() const { return getTopBottomBars(); }
    virtual juce::Colour getCabinEqTextColor() const { return getBackgroundBase(); }

    // Buy License Button
    virtual juce::Colour getBuyButtonBackground() const = 0;
    virtual juce::Colour getBuyButtonText() const { return getBackgroundBase(); }
    virtual juce::Colour getBuyButtonBorder() const { return getSecondaryAccent(); }

    // Profile Selection
    virtual juce::Colour getProfileBackground() const { return getBuyButtonBackground(); }
    virtual juce::Colour getProfileNameText() const { return getBackgroundBase(); }
    virtual juce::Colour getProfileActionText() const { return getAccentColor(); }
    virtual juce::Colour getActiveChevron() const { return getSecondaryAccent(); }
    virtual juce::Colour getInactiveChevron() const = 0;

    // EQ Graph
    virtual juce::Colour getEqGraphBackground() const { return getBackgroundBase(); }
    virtual juce::Colour getEqGraphGradientEnd() const = 0;
    virtual juce::Colour getLogLines() const { return getTextLines().withAlpha(0.1f); }
    virtual juce::Colour getEqCurveStart() const { return getSecondaryAccent(); }
    virtual juce::Colour getEqCurveMid() const = 0;
    virtual juce::Colour getEqCurveEnd() const { return getAccentColor(); }
    virtual juce::Colour getFreqLabels() const { return getTextLines(); }

    // Volume Control
    virtual juce::Colour getVolumeText() const { return getTextLines(); }
    virtual juce::Colour getVolumeSliderBackground() const { return getEqGraphGradientEnd(); }
    virtual juce::Colour getVolumeSliderForeground() const { return getSecondaryAccent(); }

    // On Button
    virtual juce::Colour getOnButtonBackground() const { return getAccentColor(); }
    virtual juce::Colour getOnButtonText() const { return getTopBottomBars(); }
    virtual juce::Colour getOnButtonBorder() const { return getSecondaryAccent(); }
};

class WoodlandMistColors : public ColorTheme
{
public:
    juce::Colour getBackgroundBase() const override          { return juce::Colour(0xfff5f2e7); }
    juce::Colour getTopBottomBars() const override           { return juce::Colour(0xff534741); }
    juce::Colour getAccentColor() const override             { return juce::Colour(0xff8aa399); }
    juce::Colour getSecondaryAccent() const override         { return juce::Colour(0xffe4b363); }
    juce::Colour getTextLines() const override               { return juce::Colour(0xff3f3a36); }
    juce::Colour getBuyButtonBackground() const override     { return juce::Colour(0xff6f5d55); }
    juce::Colour getInactiveChevron() const override         { return juce::Colour(0xffa3968f); }
    juce::Colour getEqGraphGradientEnd() const override      { return juce::Colour(0xffede9da); }
    juce::Colour getEqCurveMid() const override              { return juce::Colour(0xfff1c97a); }
};
