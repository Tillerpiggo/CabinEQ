/*
  ==============================================================================

    SIMDFilter.h
    Created: 7 Dec 2024 12:07:19am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#if JUCE_USE_SIMD
template <typename T>
static T* toBasePointer (juce::dsp::SIMDRegister<T>* r) noexcept
{
    return reinterpret_cast<T*> (r);
}

constexpr auto registerSize = juce::dsp::SIMDRegister<float>::size();

struct SIMDIIRFilter
{
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        
        iirCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 440.0f); // placeholder
        iir.reset (new juce::dsp::IIR::Filter<juce::dsp::SIMDRegister<float>> (iirCoefficients));
        
        interleaved = juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>> (interleavedBlockData, 1, spec.maximumBlockSize);
        zero = juce::dsp::AudioBlock<float> (zeroData, juce::dsp::SIMDRegister<float>::size(), spec.maximumBlockSize);
        
        zero.clear();
        
        auto monoSpec = spec;
        monoSpec.numChannels = 1;
        iir->prepare (monoSpec);
    }
    
    template <typename SampleType>
    auto prepareChannelPointers (const juce::dsp::AudioBlock<SampleType>& block)
    {
        std::array<SampleType*, registerSize> result {};
        
        for (size_t ch = 0; ch < result.size(); ++ch)
            result[ch] = (ch < block.getNumChannels() ? block.getChannelPointer (ch) : zero.getChannelPointer (ch));
        
        return result;
    }
    
    void process (const juce::dsp::ProcessContextReplacing<float>& context)
    {
        jassert (context.getInputBlock().getNumSamples() == context.getOutputBlock().getNumSamples());
        jassert (context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());
        
        const auto& input = context.getInputBlock();
        const auto numSamples = (int) input.getNumSamples();
        
        auto inChannels = prepareChannelPointers (input);
        
        using Format = juce::AudioData::Format<juce::AudioData::Float32, juce::AudioData::NativeEndian>;
        
        juce::AudioData::interleaveSamples (juce::AudioData::NonInterleavedSource<Format> { inChannels.data(),                                 registerSize, },
                                            juce::AudioData::InterleavedDest<Format>      { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                            numSamples);
        
        iir->process (juce::dsp::ProcessContextReplacing<juce::dsp::SIMDRegister<float>> (interleaved));
        
        auto outChannels = prepareChannelPointers (context.getOutputBlock());
        
        juce::AudioData::deinterleaveSamples (juce::AudioData::InterleavedSource<Format>  { toBasePointer (interleaved.getChannelPointer (0)), registerSize },
                                              juce::AudioData::NonInterleavedDest<Format> { outChannels.data(),                                registerSize },
                                              numSamples);
    }
    
    void reset()
    {
        iir.reset();
    }
    
    void setCoefficients (double sampleRate, double centerFreq, double qFactor, float amplInDB)
    {
        *iirCoefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, centerFreq, qFactor,
                                                          juce::Decibels::decibelsToGain (amplInDB));
    }
    
    //==============================================================================
    juce::dsp::IIR::Coefficients<float>::Ptr iirCoefficients;
    std::unique_ptr<juce::dsp::IIR::Filter<juce::dsp::SIMDRegister<float>>> iir;

    juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>> interleaved;
    juce::dsp::AudioBlock<float> zero;

    juce::HeapBlock<char> interleavedBlockData, zeroData;

    double sampleRate = 0.0;
};

#endif
