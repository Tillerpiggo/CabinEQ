/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "PlaybackManager.h"
#include "ClearEQValueTree.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    StartupMVPAudioProcessor();
    ~StartupMVPAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    const std::pair<juce::String, double>& getNoteData(int index) const;
    int getNoteDataSize() const;
    
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout (int numPoints);
    juce::AudioProcessorValueTreeState parameters;
    
    void applyCurve();
    void setIsBypassed (bool isBypassed);
    void setBypassBalance (float balance);
    
   
    const Curve& getCurve() const;
    const std::vector<EQNode> getEQNodes() const;
    
    // Setting points
    void addEQNode (float frequency, float amplitude, float pan);
    void removeEQNode (int id);
    void updateEQNode (int id, float frequency, float amplitude, float pan);
    void clearEQNodes();
    
    // Changing points
    void startCalibratingEQNode (EQNode eqNode);
    void updateCalibratingEQNode (EQNode eqNode);
    void endCalibratingEQNode();
    
    // Testing
    void startTestingAt (float freq);
    void updateTestingAt (float freq);
    void endTesting();
    float getCurrTestingFreq();
    
    // Sine sweep
    void startSineSweep (float centerFreq, std::optional<float> ampl = std::nullopt);
    void updateSineSweep (float centerFreq, std::optional<float> ampl = std::nullopt);
    void endSineSweep();
    float getCurrSineSweepFreq();
    
    // Green noise
    void startGreenNoise (float centerFreq);
    void updateGreenNoise (float centerFreq);
    void endGreenNoise();
    float getCurrGreenNoiseFreq();
    
    // misc
    void setReferenceVolume (float volume);
 
private:
    static const int FFT_SIZE = 10;

    PlaybackManager playbackManager;
    ClearEQValueTree clearEQValueTree;
    
    juce::dsp::ProcessSpec spec;
    
    // Hacky solution to fix gainFilter bug
    int sampleRate;
    int samplesPerBlock;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessor)
};
