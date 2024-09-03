/*
  ==============================================================================

    SineSweepGenerator.h
    Created: 28 Jul 2024 7:34:18pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SineWaveGenerator.h"
#include "Constants.h"
#include "Curve.h"

/// This class provides an easy interface to generate a sine sweep in a given frequency range.
class SineSweepGenerator
{
public:
    SineSweepGenerator();
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
//    void setCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
//    void updateCenterFrequency (float centerFreq, std::optional<float> ampl = std::nullopt);
    void setSweep (float bottomFreq, float topFreq, Curve amplCurve, Curve panCurve);
    void updateSweep (float bottomFreq, float topFreq, Curve amplCurve, Curve panCurve);
    float getCurrFreq() const;
    
private:
    void incrementFreq(); // increment frequency and amplitude and update the sine wave generator
    
    SineWaveGenerator sineWaveGenerator;
    
    float FREQ_RANGE_FACTOR = 1.5f;
    static constexpr float FREQ_STEP = 1.0001f;
    static constexpr float TEMPO = 5; // samples per change
    static constexpr float BASE_DB = 0.0f;
    bool increasingFreq = true;
    
    int currStep = 0;
    float centerFreq = REFERENCE_FREQ;
    float currFreq = REFERENCE_FREQ;
    
    Curve amplCurve;
    Curve panCurve;
    
};
