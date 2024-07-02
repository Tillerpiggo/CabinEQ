/*
  ==============================================================================

    Curve.h
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CalibratedSetPoint.h"

// This manages a curve interpolated between a list of set points, with an arbitrary resolution.
class Curve
{
public:
    Curve (const std::vector<float>& frequencies, 
           const std::vector<CalibratedSetPoint>& amplitudes,
           const std::vector<CalibratedSetPoint>& phases,
           const std::vector<CalibratedSetPoint>& pans)
    : frequencies (frequencies), amplitudes (amplitudes), phases (phases), pans(pans)
    {
//        std::cout << "Frequencies: ";
//        for (const auto& frequency : frequencies) {
//            std::cout << frequency << " ";
//        }
//        std::cout << std::endl;
//
//        std::cout << "Amplitudes: ";
//        for (const auto& amplitude : amplitudes) {
//            std::cout << amplitude.estimatedValue() << " ";
//        }
//        std::cout << std::endl;
//
//        std::cout << "Phases: ";
//        if (phases) {
//            for (const auto& phase : *phases) {
//                std::cout << phase.estimatedValue() << " ";
//            }
//        }
//        std::cout << std::endl;
//
//        std::cout << "Pans: ";
//        if (pans) {
//            for (const auto& pan : *pans) {
//                std::cout << pan.estimatedValue() << " ";
//            }
//        }
//        std::cout << std::endl;
    }
    
    virtual ~Curve() {}
    
    const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtNormalizedTime (float time) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const float cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const;
    
    void setFactor (const float factor)
    {
        this->factor = factor;
    }
    
protected:
    const float interpolateValueAtFrequency (const float frequency, 
                                             const std::vector<CalibratedSetPoint>& values) const;
    const std::vector<float>& frequencies;
    const std::vector<CalibratedSetPoint>& amplitudes;
    const std::vector<CalibratedSetPoint>& phases;
    const std::vector<CalibratedSetPoint>& pans;
    float factor = 1.f;
};
