/////*
////  ==============================================================================
////
////    Curve.cpp
////    Created: 13 Jun 2024 8:25:42pm
////    Author:  Tyler Gee
////
////  ==============================================================================
////*/
////
//
///*
//  ==============================================================================
//
//    Curve.cpp
//    Created: 13 Jun 2024 8:25:42pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#include "Curve.h"
//
//const float Curve::valueAtFrequency (float frequency) const
//{
//    // Create amplitudes from eqNodes
//    std::vector<float> values;
//    for (const auto& curvePt : curvePts)
//    {
//        values.push_back (curvePt.val);
//    }
//    
//    float valueAtFrequency = interpolateValueAtFrequency (frequency, values);
////    if (frequency > 20.0f)
////        valueAtFrequency *= juce::Decibels::decibelsToGain (-3.0 * std::log2 (frequency / 1000.0f));
//    return valueAtFrequency;
//}
//
//const float Curve::visualValueAtFrequency (float frequency)
//{
//    if (cache.find(frequency) != cache.end())
//    {
//        return cache[frequency];
//    }
//    
//    std::vector<float> values;
//    for (const auto& curvePt : curvePts)
//    {
//        values.push_back (curvePt.val);
//    }
//    
//    float valueAtFrequency = visualInterpolateAmplitudeAtFrequency(frequency);
//    
//    return valueAtFrequency;
//}
//
//const float Curve::valueAtTime (float t)
//{
//    return valueAtFrequency (t * 22050);
//}
//
//const std::vector<CurvePt>& Curve::getCurvePts()
//{
//    return curvePts;
//}
//
//float Curve::catmullRom(float t, float y0, float y1, float y2, float y3) const
//{
//    float t2 = t * t;
//    float t3 = t * t * t;
//    float y = 0.5f * ((2.f * y1) + (-y0 + y2) * t + (2.f * y0 - 5.f * y1 + 4.f * y2 - y3) * t2 + (-y0 + 3.f * y1 - 3.f * y2 + y3) * t3);
//    return y;
//}
//
//void Curve::updateWithCurvePts (std::vector<CurvePt> curvePts)
//{
//    std::sort(curvePts.begin(), curvePts.end(), [](const CurvePt &a, const CurvePt &b)
//    {
//        return a.freq < b.freq;
//    });
//    
//    this->curvePts = curvePts;
//    cache.clear();
//}
//
///*
//const float* Curve::getImpulse (int fft_size)
//{
//    // Perform an IFFT on the desired frequency response
//    juce::dsp::FFT fft (fft_size);
//    int numPoints = fft.getSize();
//    
//    auto freqResponse = frequencyResponse (numPoints);
//    auto leftFreqResponse = freqResponse.first;
//    auto rightFreqResponse = freqResponse.second;
//
//    fft.performRealOnlyInverseTransform (leftFreqResponse);
//    fft.performRealOnlyInverseTransform (rightFreqResponse);
//    
//    float* leftImpulseData = leftFreqResponse;
//    float* rightImpulseData = rightFreqResponse;
//    
//    for (int i = 0; i < numPoints * 2; ++i)
//    {
//        leftImpulseData[i] = leftFreqResponse[i];
//        rightImpulseData[i] = rightFreqResponse[i];
//    }
//
//    // Transform post-ringing into pre-ringing
//    for (int i = 0; i < numPoints / 2; ++i)
//    {
//        std::swap(leftImpulseData[i], leftImpulseData[i + numPoints / 2]);
//        std::swap(rightImpulseData[i], rightImpulseData[i + numPoints / 2]);
//    }
//    
//    // Window the impulse
//    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::rectangular, true);
//    window.multiplyWithWindowingTable(leftImpulseData, numPoints);
//    window.multiplyWithWindowingTable(rightImpulseData, numPoints);
//    
//    return { leftImpulseData, rightImpulseData };
//}
// */
//
//const float Curve::interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const
//{
//    size_t numPoints = curvePts.size();
//    
//    // Edge case checks
//    if (curvePts.size() == 0) return 0.0f;
//    if (frequency < curvePts.at (0).freq) return values.at (0);
//    if (frequency > curvePts.at(numPoints - 1).freq) return values.at (numPoints - 1);
//    
//    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
//    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
//    
//    for (size_t i = 0; i < numPoints; ++i)
//    {
//        float currFreq = curvePts.at (i).freq;
//        if (frequency == currFreq)
//        {
//            return values.at(i);
//        }
//        
//        if (frequency < currFreq)
//        {
//            freq1 = curvePts.at(i - 1).freq;
//            gain1 = values.at(i - 1);
//            freq2 = curvePts.at(i).freq;
//            gain2 = values.at(i);
//            
//            gain0 = (i > 1) ? values.at (i - 2) : gain1;
//            freq0 = (i > 1) ? curvePts.at (i - 2).freq : freq1;
//            gain3 = (i < numPoints - 1) ? values.at (i + 1) : gain2;
//            freq3 = (i < numPoints - 1) ? curvePts.at (i + 1).freq : freq2;
//            
//            break;
//        }
//    }
//    
//    float logMinFreq = std::log(freq1);
//    float logMaxFreq = std::log(freq2);
//    float logFreq = std::log(frequency);
//
//    // Normalize the log frequency
//    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
//    float gainAtFrequency = catmullRom (t, gain0, gain1, gain2, gain3);
//    
//    return gainAtFrequency;
//}
//
//const float Curve::visualInterpolateAmplitudeAtFrequency (const float frequency) const
//{
//    size_t numPoints = curvePts.size();
//    
//    auto logCompensation = [](float freq) { return 0.0f; };//-4.5 * std::log2(freq / 1000); };
//    
//    // Edge case checks
//    if (curvePts.size() == 0) return logCompensation (frequency);
//    if (frequency < curvePts.at(0).freq) return curvePts.at(0).val + logCompensation (frequency) - logCompensation (curvePts.at(0).freq);
//    if (frequency > curvePts.at(numPoints - 1).freq) return curvePts.at(numPoints - 1).val + logCompensation (frequency) - logCompensation (curvePts.at(numPoints - 1).freq);
//    
//    float freq0 = 0, freq1 = 0, freq2 = 0, freq3 = 0;
//    float gain0 = 0, gain1 = 0, gain2 = 0, gain3 = 0;
//    
//    for (size_t i = 0; i < numPoints; ++i)
//    {
//        float currFreq = curvePts.at(i).freq;
//        if (frequency == currFreq)
//        {
//            return curvePts.at(i).val;
//        }
//        
//        if (frequency < currFreq)
//        {
//            freq1 = curvePts.at(i - 1).freq;
//            gain1 = curvePts.at(i - 1).val - logCompensation(freq1);
//            freq2 = curvePts.at(i).freq;
//            gain2 = curvePts.at(i).val - logCompensation(freq2);
//            
//            freq0 = (i > 1) ? curvePts.at(i - 2).freq : freq1 / 2;
//            gain0 = (i > 1) ? curvePts.at(i - 2).val - logCompensation(freq0) : gain1;
//            freq3 = (i < numPoints - 1) ? curvePts.at(i + 1).freq : freq2 * 2;
//            gain3 = (i < numPoints - 1) ? curvePts.at(i + 1).val - logCompensation(freq3) : gain2;
//            
//            break;
//        }
//    }
//    
//    float logMinFreq = std::log(freq1);
//    float logMaxFreq = std::log(freq2);
//    float logFreq = std::log(frequency);
//
//    // Normalize the log frequency
//    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
//    float gainAtFrequency = catmullRom(t, gain0, gain1, gain2, gain3);
//    
//    return gainAtFrequency + logCompensation (frequency);
//}
//
//std::vector<float> Curve::getFrequencyResponse (int numPoints)
//{
//    float maxFreq = 60.0f;
//    float minFreq = -48.0f;
//    
//    std::vector<float> freqResponse (2 * numPoints, 0);
//    /*
//    for (int i = 0; i < numPoints; ++i)
//    {
//        float t = static_cast<float>(i) / (numPoints);
//        
//        auto val = juce::Decibels::decibelsToGain (valueAtTime (t));
//        
//        if (i % 2 == 0)
//        {
//            if (val > maxFreq) val = maxFreq;
//            if (val < minFreq) val = minFreq;
//            freqResponse[i] = val;
//        }
//        else
//        {
//            freqResponse[i] = 0;
//        }
//    }
//    
//    for (int i = 0; i < numPoints; ++i)
//    {
//        float t = static_cast<float>(i) / (numPoints);
//        
//        auto val = juce::Decibels::decibelsToGain (valueAtTime (1 - t));
//        
//        if (i % 2 == 0)
//        {
//            if (val > maxFreq) val = maxFreq;
//            if (val < minFreq) val = minFreq;
//            freqResponse[i + numPoints] = val;
//        }
//        else
//        {
//            freqResponse[i] = 0;
//        }
//    }
//     */
//    for (int i = 0; i < numPoints; ++i)
//    {
//        float t = static_cast<float>(i) / (numPoints);
//        
//        auto val = juce::Decibels::decibelsToGain (valueAtTime (t));
//        
//        if (i % 2 == 0)
//        {
//            if (val > maxFreq) val = maxFreq;
//            if (val < minFreq) val = minFreq;
//            freqResponse[i] = val;
//        }
//        else
//        {
//            freqResponse[i] = 0;
//        }
//    }
//    
//    for (int i = 0; i < numPoints / 2; ++i)
//    {
//        freqResponse[2 * i + numPoints] = freqResponse[numPoints - 2 * (i + 1)];
//        freqResponse[2 * i + numPoints + 1] = freqResponse[numPoints - 2 * (i + 1) + 1];
//    }
//    
//    /*
//    for (int i = 0; i < numPoints; ++i)
//    {
//        float t = static_cast<float>(i) / (numPoints);
//        
//        auto val = juce::Decibels::decibelsToGain (valueAtTime (1 - t));
//        
//        if (i % 2 == 0)
//        {
//            if (val > maxFreq) val = maxFreq;
//            if (val < minFreq) val = minFreq;
//            freqResponse[i + numPoints] = val;
//        }
//        else
//        {
//            freqResponse[i] = 0;
//        }
//    }
//     */
//    
//    return freqResponse;
//}
//
//const std::optional<std::pair<float, float>> Curve::nodeBelowFreq (float frequency)
//{
//    int numNodes = static_cast<int> (curvePts.size());
//    for (int i = numNodes - 1; i >= 0; --i)
//    {
//        if (curvePts[i].freq < frequency)
//        {
//            float freq = curvePts[i].freq;
//            float val = curvePts[i].val;
//            return std::optional<std::pair<float, float>> ({ freq, val });
//        }
//    }
//    
//    return std::nullopt;
//}
//
//const std::optional<std::pair<float, float>> Curve::nodeAboveFreq (float frequency)
//{
//    for (int i = 0; i < curvePts.size(); ++i)
//    {
//        if (curvePts[i].freq > frequency)
//        {
//            float freq = curvePts[i].freq;
//            float val = curvePts[i].val;
//            return std::optional<std::pair<float, float>> ({ freq, val });
//        }
//    }
//    
//    return std::nullopt;
//}
//
//const std::optional<std::vector<float>> Curve::getFirstThreeFreqs()
//{
//    std::vector<float> freqs;
//    
//    for (const auto& curvePt : curvePts)
//    {
//        if (curvePt.id < 3)
//        {
//            freqs.push_back (curvePt.freq);
//        }
//    }
//    
//    if (freqs.size() == 3)
//    {
//        return freqs;
//    }
//    else
//    {
//        return std::nullopt;
//    }
//}
//
//const std::optional<std::vector<float>> Curve::getFirstFourFreqs()
//{
//    std::vector<float> freqs;
//    
//    for (const auto& curvePt : curvePts)
//    {
//        if (curvePt.id < 4)
//        {
//            freqs.push_back (curvePt.freq);
//        }
//    }
//    
//    if (freqs.size() == 4)
//    {
//        return freqs;
//    }
//    else
//    {
//        return std::nullopt;
//    }
//}
//
//std::pair<std::vector<float>, std::vector<float>> Curve::getStereoFrequencyResponse (Curve& amplCurve, /*Curve& panCurve, Curve& phaseCurve,*/ int numPoints)
//{
//
//    std::vector<float> leftFreqResponse (2 * numPoints, 0.0f);
//    std::vector<float> rightFreqResponse (2 * numPoints, 0.0f);
//
//    // Compute the positive frequencies (including DC and Nyquist)
//    for (int i = 0; i <= numPoints / 2; ++i)
//    {
//        float t = static_cast<float>(i) / (numPoints / 2);
//
//        float ampl = amplCurve.valueAtTime (t);
////        float pan = panCurve.valueAtTime (t);
////        float phase = phaseCurve.valueAtTime (t);
//        
//        float leftGain = juce::Decibels::decibelsToGain (ampl);
//        float rightGain = juce::Decibels::decibelsToGain (ampl);
//
////        float leftGain = juce::Decibels::decibelsToGain (ampl - (pan < 0 ? pan : 0));
////        float rightGain = juce::Decibels::decibelsToGain (ampl + (pan > 0 ? pan : 0));
//        
//        std::complex<float> leftComplexVal = std::polar (leftGain, 0.0f);
//        std::complex<float> rightComplexVal = std::polar (rightGain, 0.0f);
//
//        // Compute the complex frequency response using magnitude and phase
////        std::complex<float> leftComplexVal = std::polar (leftGain, -0.5f * phase);
////        std::complex<float> rightComplexVal = std::polar (rightGain, 0.5f * phase);
//
//        leftFreqResponse[2 * i] = leftComplexVal.real();
//        leftFreqResponse[2 * i + 1] = leftComplexVal.imag();
//
//        rightFreqResponse[2 * i] = rightComplexVal.real();
//        rightFreqResponse[2 * i + 1] = rightComplexVal.imag();
//    }
//
//    // Compute the negative frequencies by ensuring conjugate symmetry
//    for (int i = 1; i < numPoints / 2; ++i)
//    {
//        int reverseIdx = numPoints - i;
//
//        // Conjugate symmetry for the left channel
//        leftFreqResponse[2 * reverseIdx] = leftFreqResponse[2 * i];
//        leftFreqResponse[2 * reverseIdx + 1] = -leftFreqResponse[2 * i + 1];
//
//        // Conjugate symmetry for the right channel
//        rightFreqResponse[2 * reverseIdx] = rightFreqResponse[2 * i];
//        rightFreqResponse[2 * reverseIdx + 1] = -rightFreqResponse[2 * i + 1];
//    }
//
//    return { leftFreqResponse, rightFreqResponse };
//}


/*
  ==============================================================================

    Curve.cpp
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Curve.h"

const float Curve::valueAtTime (float t)
{
    float freq = t * 22050;
    float dbOffset = -3.0f * std::log2 (std::max (freq, 20.0f) / 1000.0f);
    return dbOffset;
}
/*
const float* Curve::getImpulse (int fft_size)
{
    // Perform an IFFT on the desired frequency response
    juce::dsp::FFT fft (fft_size);
    int numPoints = fft.getSize();
    
    auto freqResponse = frequencyResponse (numPoints);
    auto leftFreqResponse = freqResponse.first;
    auto rightFreqResponse = freqResponse.second;

    fft.performRealOnlyInverseTransform (leftFreqResponse);
    fft.performRealOnlyInverseTransform (rightFreqResponse);
    
    float* leftImpulseData = leftFreqResponse;
    float* rightImpulseData = rightFreqResponse;
    
    for (int i = 0; i < numPoints * 2; ++i)
    {
        leftImpulseData[i] = leftFreqResponse[i];
        rightImpulseData[i] = rightFreqResponse[i];
    }

    // Transform post-ringing into pre-ringing
    for (int i = 0; i < numPoints / 2; ++i)
    {
        std::swap(leftImpulseData[i], leftImpulseData[i + numPoints / 2]);
        std::swap(rightImpulseData[i], rightImpulseData[i + numPoints / 2]);
    }
    
    // Window the impulse
    juce::dsp::WindowingFunction<float> window(numPoints, juce::dsp::WindowingFunction<float>::rectangular, true);
    window.multiplyWithWindowingTable(leftImpulseData, numPoints);
    window.multiplyWithWindowingTable(rightImpulseData, numPoints);
    
    return { leftImpulseData, rightImpulseData };
}
 */
std::vector<float> Curve::getFrequencyResponse (int numPoints)
{
    float maxFreq = 60.0f;
    float minFreq = -48.0f;
    
    std::vector<float> freqResponse (2 * numPoints, 0);
    
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints);
        
        auto val = juce::Decibels::decibelsToGain (valueAtTime (t));
        
        if (i % 2 == 0)
        {
            if (val > maxFreq) val = maxFreq;
            if (val < minFreq) val = minFreq;
            freqResponse[i] = val;
        }
        else
        {
            freqResponse[i] = 0;
        }
    }
    
    for (int i = 0; i < numPoints / 2; ++i)
    {
        freqResponse[2 * i + numPoints] = freqResponse[numPoints - 2 * (i + 1)];
        freqResponse[2 * i + numPoints + 1] = freqResponse[numPoints - 2 * (i + 1) + 1];
    }
    
    return freqResponse;
}

std::pair<std::vector<float>, std::vector<float>> Curve::getStereoFrequencyResponse (Curve& amplCurve, int numPoints)
{
    std::vector<float> leftFreqResponse (2 * numPoints, 0.0f);
    std::vector<float> rightFreqResponse (2 * numPoints, 0.0f);

    // Compute the positive frequencies (including DC and Nyquist)
    for (int i = 0; i <= numPoints / 2; ++i)
    {
        float t = static_cast<float>(i) / (numPoints / 2);

        float ampl = amplCurve.valueAtTime (t);
        float gain = juce::Decibels::decibelsToGain (ampl);

        leftFreqResponse[2 * i] = gain;
        leftFreqResponse[2 * i + 1] = 0;

        rightFreqResponse[2 * i] = gain;
        rightFreqResponse[2 * i + 1] = 0;
    }

    // Compute the negative frequencies by ensuring conjugate symmetry
    for (int i = 1; i < numPoints / 2; ++i)
    {
        int reverseIdx = numPoints - i;

        // Conjugate symmetry for the left channel
        leftFreqResponse[2 * reverseIdx] = leftFreqResponse[2 * i];
        leftFreqResponse[2 * reverseIdx + 1] = -leftFreqResponse[2 * i + 1];

        // Conjugate symmetry for the right channel
        rightFreqResponse[2 * reverseIdx] = rightFreqResponse[2 * i];
        rightFreqResponse[2 * reverseIdx + 1] = -rightFreqResponse[2 * i + 1];
    }

    return { leftFreqResponse, rightFreqResponse };
}
