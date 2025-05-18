/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "PlaybackManager.h"
#include "ArbitraryResponseFilter.h"
#include <cmath>
#include <random>

PlaybackManager::PlaybackManager()
    : firFilter (14),
//      tiltFilter (12),
      isFilterOn (true),
      isPlayingNoise (false)
{
    profileVolumeProcessor.setRampDurationSeconds (0.05);
    profileVolumeProcessor.setGainDecibels (0.0f);
//    systemVolumeProcessor.setRampDurationSeconds (0.05);
//    systemVolumeProcessor.setGain (juce::SystemAudioVolume::getGain());
    overallVolumeProcessor.setRampDurationSeconds (0.05);
    overallVolumeProcessor.setGainDecibels (0.0f);
    
    audioFormatManager.registerBasicFormats();
    audioTransportSource.addChangeListener (this);
    crossfeedProcessor.setEnabled(false); // Default to off
    crossfeedProcessor.setCrossfeedVolume(0.0f); // Default to off
    
    startTimer (50);
}

void PlaybackManager::processBlock (juce::AudioBuffer<float>& ioBuffer)
{
    if (isPlayingNoise)
    {
//         checkerboardPlayer.processBlock (ioBuffer, systemVolume * juce::Decibels::decibelsToGain (calibrationVolume));
        glyphGridPlayer.processBlock (ioBuffer, systemVolume * juce::Decibels::decibelsToGain (calibrationVolume));
    }
    
    juce::dsp::AudioBlock<float> ioBlock (ioBuffer);
    juce::dsp::ProcessContextReplacing<float> ioContext (ioBlock);
    
    if (isFilterOn)
    {
        if (isIIR)
        {
            filter.process (ioBlock);
        }
        else
        {
            firFilter.process (ioContext);
        }

        crossfeedProcessor.process(ioBlock);
        
        profileVolumeProcessor.process (ioContext);
        
        if (isProvisionalOn)
        {
            provisionalFilter.process (ioBlock);
        }
    }
    
    overallVolumeProcessor.process (ioContext);
}

void PlaybackManager::updateFilterWithBandProfile (BandProfile bandProfile)
{
    this->bandEqCurve.updateWithBands (bandProfile.getBands());
    filter.setBands (bandProfile.getBands(), spec.sampleRate);
    profileVolumeProcessor.setGainDecibels (bandProfile.getVolume());
}

void PlaybackManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->spec = spec;
    
    audioTransportSource.prepareToPlay (spec.maximumBlockSize, spec.sampleRate);
    checkerboardPlayer.prepare (spec);
    glyphGridPlayer.prepare (spec);
    
    filter.prepare (spec);
    provisionalFilter.prepare (spec);
    firFilter.prepare (spec);
    crossfeedProcessor.prepare(spec);
//    tiltFilter.prepare (spec);
//    tiltFilter.updateWithCurve (tiltCurve, 12);
//    firFilter.updateWithCurve (firCurve);
}


void PlaybackManager::setIsFilterOn (bool isFilterOn)
{
    this->isFilterOn = isFilterOn;
}

void PlaybackManager::setIsPlayingNoise (bool isPlayingNoise)
{
    this->isPlayingNoise = isPlayingNoise;
}

void PlaybackManager::setIsCabinNoise (bool isCabinNoise)
{
    this->isCabinNoise = isCabinNoise;
}

void PlaybackManager::setVolume (float volume)
{
    this->volume = volume;
    overallVolumeProcessor.setGainDecibels (volume);
}

void PlaybackManager::setCalibrationVolume (float calibrationVolume)
{
    this->calibrationVolume = calibrationVolume;
}

void PlaybackManager::setMinFreq (float minFreq)
{
    checkerboardPlayer.setMinFreq (minFreq);
}

void PlaybackManager::setPinkNoise (bool pinkNoiseEnabled)
{
    this->isCabinNoise = ! pinkNoiseEnabled;
}

void PlaybackManager::updateFIRFilter()
{
    // Calculate curve pts
    std::vector<CurvePt> leftCurvePts;
    std::vector<CurvePt> rightCurvePts;
    const float startFreq = 20.0f;
    const float endFreq = 20000.0f;
    const int numPoints = 4000;
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = startFreq * std::pow (endFreq / startFreq, i / (numPoints - 1.0f));
        float leftAmpl = bandEqCurve.leftDbAtFrequency (freq);
        float rightAmpl = bandEqCurve.rightDbAtFrequency (freq);
        leftCurvePts.push_back (CurvePt (i, freq, leftAmpl));
        rightCurvePts.push_back (CurvePt (i, freq, rightAmpl));
    }
    
    firCurve.updateWithCurvePts (leftCurvePts, rightCurvePts);
    firFilter.updateWithCurve (firCurve, fftSize);
}

void PlaybackManager::setFIRQuality (int fftSize)
{
    this->fftSize = fftSize;
}

void PlaybackManager::setIIR (bool isIIR)
{
    this->isIIR = isIIR;
}

void PlaybackManager::setProvisionalBands (std::vector<Band> provisionalBands)
{
    provisionalFilter.setBands (provisionalBands, spec.sampleRate);
}

void PlaybackManager::setProvisionalBandsOn (bool provisionalBandsOn)
{
    this->isProvisionalOn = provisionalBandsOn;
}

void PlaybackManager::setCheckerboard (Checkerboard checkerboard)
{
    checkerboardPlayer.setCheckerboard (checkerboard);
}

void PlaybackManager::setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords)
{
    checkerboardPlayer.setSoloSquareCoords (soloSquareCoords);
}

void PlaybackManager::setSpeedFactor (float speedFactor)
{
    glyphGridPlayer.setSpeedFactor (speedFactor);
}

void PlaybackManager::setBandwidth (float bandwidth)
{
    glyphGridPlayer.setBandwidth (bandwidth);
}

void PlaybackManager::setGlyphs (std::vector<Glyph> glyphs)
{
    glyphGridPlayer.setGlyphs (glyphs);
}

void PlaybackManager::setGlyphVolume(float volume)
{
    glyphGridPlayer.setVolume(volume);
}

float PlaybackManager::getCurrPlayingTime()
{
    return glyphGridPlayer.getCurrPlayingTime();
}

void PlaybackManager::setIsAudioFilePlaying (bool isPlaying)
{
    if (isPlaying)
    {
        audioTransportSource.start();
    }
    else
    {
        audioTransportSource.stop();
    }
}

void PlaybackManager::setListener (PlaybackManagerListener* listener)
{
    this->listener = listener;
}

void PlaybackManager::setAudioFile (juce::File file)
{
    auto* fileReader = audioFormatManager.createReaderFor (file);
    
    if (fileReader != nullptr)
    {
        auto newAudioSource = std::make_unique<juce::AudioFormatReaderSource> (fileReader, true);
        audioTransportSource.setSource (newAudioSource.get(), 0, nullptr, fileReader->sampleRate);
        if (listener != nullptr)
        {
//            listener->fileDidLoad();
        }
        audioReaderSource.reset (newAudioSource.release());
    }
}

bool PlaybackManager::getIsPlaying()
{
    return isPlayingNoise;
}

float PlaybackManager::getBandwidth()
{
    return bandwidth;
}

std::vector<float> PlaybackManager::getCurrPlayingFreqs()
{
//    return checkerboardPlayer.getCurrSolodFreqs();
   // return {}; // for checkerboard player
    return glyphGridPlayer.getCurrPlayingFreqs();
//    return
//    return gridSequencer.getCurrPlayingFreqs();
}

std::vector<std::pair<float, float>> PlaybackManager::getCurrPlayingFreqsAndVols()
{
//    return {}; // for checkerboard player
    return glyphGridPlayer.getCurrPlayingFreqsAndVols();
}

void PlaybackManager::timerCallback()
{
    systemVolume = juce::SystemAudioVolume::isMuted() ? 0.0f : juce::SystemAudioVolume::getGain();
}

void PlaybackManager::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &audioTransportSource)
    {
        listener->audioFilePlayingChanged (audioTransportSource.isPlaying());
    }
}

std::pair<float, float> PlaybackManager::getNextSample()
{
//     return checkerboardPlayer.getNextSample();
    auto sample = glyphGridPlayer.getNextSample();
//    std::cout << "sample: " << sample.first << ", " << sample.second << std::endl;
    
//    return sample;
//    return gridSequencer.getNextSample();
}

void PlaybackManager::setCrossfeedDelaySamples (int samples)
{
    crossfeedProcessor.setDelaySamples(samples);
}

void PlaybackManager::setCrossfeedVolume (float volume)
{
    crossfeedProcessor.setCrossfeedVolume(volume);
}

void PlaybackManager::setCrossfeedEnabled (bool enabled)
{
    crossfeedProcessor.setEnabled(enabled);
}
