#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

// The pink noise CabinEQ System plays (from another process) to check audio really goes
// tap -> plugin -> speakers. Used by the automated test and the in-app test sound.
namespace TestSignal
{
    inline bool writeWav (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate, int numSamples)
    {
        file.deleteFile();
        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());

        if (stream == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (wav.createWriterFor (stream.get(), sampleRate, (unsigned) buffer.getNumChannels(), 24, {}, 0));

        if (writer == nullptr)
            return false;

        stream.release();   // the writer owns it now
        return writer->writeFromAudioSampleBuffer (buffer, 0, numSamples);
    }

    inline bool writePinkNoise (const juce::File& file, double seconds, float levelDb, double sampleRate = 48000.0)
    {
        const int numSamples = (int) (seconds * sampleRate);
        juce::AudioBuffer<float> buffer (2, numSamples);
        juce::Random random (1234);
        float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0;

        for (int i = 0; i < numSamples; ++i)
        {
            // Paul Kellet's pink noise filter
            const float white = random.nextFloat() * 2.0f - 1.0f;
            b0 = 0.99886f * b0 + white * 0.0555179f;
            b1 = 0.99332f * b1 + white * 0.0750759f;
            b2 = 0.96900f * b2 + white * 0.1538520f;
            b3 = 0.86650f * b3 + white * 0.3104856f;
            b4 = 0.55000f * b4 + white * 0.5329522f;
            b5 = -0.7616f * b5 - white * 0.0168980f;
            buffer.setSample (0, i, b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f);
            b6 = white * 0.115926f;
        }

        const int fade = (int) (0.05 * sampleRate);
        buffer.applyGain (0, 0, numSamples, juce::Decibels::decibelsToGain (levelDb) / buffer.getRMSLevel (0, 0, numSamples));
        buffer.applyGainRamp (0, 0, fade, 0.0f, 1.0f);
        buffer.applyGainRamp (0, numSamples - fade, fade, 1.0f, 0.0f);
        buffer.copyFrom (1, 0, buffer, 0, 0, numSamples);

        return writeWav (file, buffer, sampleRate, numSamples);
    }
}
