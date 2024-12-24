#pragma once

#include <juce_dsp/juce_dsp.h>

// Alias for SIMD data type (typically holds 4 floats on SSE)
using SIMDType = juce::dsp::SIMDRegister<float>;

class SIMDIIRFilter
{
public:
    SIMDIIRFilter() {}

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    void reset()
    {
        // Reset state variables
        z1 = SIMDType(0.0f);
        z2 = SIMDType(0.0f);
        y1 = SIMDType(0.0f);
        y2 = SIMDType(0.0f);
    }

    void setCoefficients(const juce::dsp::IIR::Coefficients<float>::Ptr& coefficients)
    {
        // Load coefficients into SIMD registers (broadcast)
        b0 = SIMDType(coefficients->coefficients[0]);
        b1 = SIMDType(coefficients->coefficients[1]);
        b2 = SIMDType(coefficients->coefficients[2]);
        a1 = SIMDType(coefficients->coefficients[3]);
        a2 = SIMDType(coefficients->coefficients[4]);
    }

    void process(juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        auto numSamples = inputBlock.getNumSamples();
        auto numChannels = inputBlock.getNumChannels();

        // Ensure we're processing a single channel
        jassert(numChannels == 1);

        auto* channelData = outputBlock.getChannelPointer(0);

        // Process samples
        for (size_t i = 0; i < numSamples; ++i)
        {
            // Load sample into SIMD register (broadcast to all lanes)
            SIMDType input(channelData[i]);

            // Apply the IIR filter equation
            SIMDType output = b0 * input + b1 * z1 + b2 * z2 - a1 * y1 - a2 * y2;

            // Store the output sample (accessing the first lane)
            channelData[i] = output[0];

            // Update the state variables (delay lines)
            z2 = z1;
            z1 = input;
            y2 = y1;
            y1 = output;
        }
    }

private:
    double sampleRate = 44100.0;

    // Filter coefficients (broadcasted to all SIMD lanes)
    SIMDType b0, b1, b2;
    SIMDType a1, a2;

    // State variables (delay lines)
    SIMDType z1, z2;
    SIMDType y1, y2;
};
