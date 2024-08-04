/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <chrono>

//==============================================================================
StartupMVPAudioProcessor::StartupMVPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          headphoneEQValueTree (parameters, "HeadphoneEQ"),
                          speakerEQValueTree (parameters, "SpeakerEQ"),
                          activeCurveId (HEADPHONE_EQ_ID)

#endif
{
    
}

StartupMVPAudioProcessor::~StartupMVPAudioProcessor()
{
}

//==============================================================================
const juce::String StartupMVPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StartupMVPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StartupMVPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StartupMVPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StartupMVPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StartupMVPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StartupMVPAudioProcessor::getProgramName (int index)
{
    return {};
}

void StartupMVPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StartupMVPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
}

void StartupMVPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StartupMVPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StartupMVPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear buffer before handing it off to playbackManager
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    playbackManager.processBlock (buffer);
    /*
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    if (playbackManager.getIsCalibrating())
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = playbackManager.getNextSample();
            leftChannel[sample] = value.first * 0.05 * 0.5;
            
            if (rightChannel)
                rightChannel[sample] = value.second * 0.05 * 0.5;
        }
    }
    else
    {
        // process audio through the filter
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        if (isBypassed && hasPreparedFilter)
        {
//            gainFilter.process (context);
            // TODO: Fix this
            wetGainProcessor.process (context);
        }
        else
        {
            dryGainProcessor.process (context);
        }
    }
     */
}

//==============================================================================
bool StartupMVPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StartupMVPAudioProcessor::createEditor()
{
    return new StartupMVPAudioProcessorEditor (*this);
}

//==============================================================================
void StartupMVPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StartupMVPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get())
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            headphoneEQValueTree.initValueTreeFromAPVTS();
            speakerEQValueTree.initValueTreeFromAPVTS();
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StartupMVPAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout StartupMVPAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void StartupMVPAudioProcessor::applyCurve()
{
    if (activeCurveId == HEADPHONE_EQ_ID)
        playbackManager.updateFilterWithCurve (headphoneEQValueTree.getCurve());
    else
        playbackManager.updateFilterWithCurve (speakerEQValueTree.getCurve());
}

void StartupMVPAudioProcessor::setIsBypassed (bool isBypassed)
{
    if (isBypassed) // isBypassed = true means the filter is being applied. I should probably rename this at some point.
        applyCurve();
    playbackManager.setIsBypassed (isBypassed);
}

void StartupMVPAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

Curve& StartupMVPAudioProcessor::getCurve (juce::String curveId)
{
    if (curveId == HEADPHONE_EQ_ID)
    {
        return headphoneEQValueTree.getCurve();
    }
    else
    {
        return speakerEQValueTree.getCurve();
    }
}

void StartupMVPAudioProcessor::addEQNode (float frequency, float amplitude, float pan, juce::String curveId)
{
    if (curveId == HEADPHONE_EQ_ID)
        headphoneEQValueTree.addEQNode (frequency, amplitude, pan);
    else
        speakerEQValueTree.addEQNode (frequency, amplitude, pan);
}

void StartupMVPAudioProcessor::removeEQNode (int id, juce::String curveId)
{
    if (curveId == HEADPHONE_EQ_ID)
        headphoneEQValueTree.removeEQNode (id);
    else
        speakerEQValueTree.removeEQNode (id);
}

void StartupMVPAudioProcessor::updateEQNode (int id, float frequency, float amplitude, float pan, juce::String curveId)
{
    if (curveId == HEADPHONE_EQ_ID)
        headphoneEQValueTree.updateEQNode (id, frequency, amplitude, pan);
    else
        speakerEQValueTree.updateEQNode (id, frequency, amplitude, pan);
}

void StartupMVPAudioProcessor::clearEQNodes (juce::String curveId)
{
    if (curveId == HEADPHONE_EQ_ID)
        headphoneEQValueTree.resetNodes ({});
    else
        speakerEQValueTree.resetNodes ({});
}

void StartupMVPAudioProcessor::startCalibratingEQNode (EQNode node)
{
    playbackManager.setCalibratingEQNode (node);
    playbackManager.setIsCalibrating (true);
}

void StartupMVPAudioProcessor::updateCalibratingEQNode (EQNode node)
{
    playbackManager.updateCalibratingEQNode (node);
}

void StartupMVPAudioProcessor::endCalibratingEQNode()
{
    playbackManager.setIsCalibrating (false);
}

void StartupMVPAudioProcessor::startTestingAt (float freq, juce::String curveId)
{
    playbackManager.startTestingFreq (freq, getCurve (curveId));
}

void StartupMVPAudioProcessor::updateTestingAt (float freq, juce::String curveId)
{
    playbackManager.updateTestingFreq (freq, getCurve (curveId));
}

void StartupMVPAudioProcessor::endTesting()
{
    playbackManager.stopTestingFreq();
}

float StartupMVPAudioProcessor::getCurrTestingFreq()
{
    return playbackManager.getCurrTestingFreq();
}

const std::vector<EQNode> StartupMVPAudioProcessor::getEQNodes (juce::String curveId) const
{
    if (curveId == HEADPHONE_EQ_ID)
        return headphoneEQValueTree.getEQNodes();
    else
        return speakerEQValueTree.getEQNodes();
}

void StartupMVPAudioProcessor::startSineSweep (float centerFreq, std::optional<float> ampl)
{
    playbackManager.setIsSweeping (true);
    playbackManager.setSineSweepCenterFrequency (centerFreq, ampl);
}

void StartupMVPAudioProcessor::updateSineSweep (float centerFreq, std::optional<float> ampl)
{
    playbackManager.updateSineSweepCenterFrequency (centerFreq, ampl);
}

void StartupMVPAudioProcessor::endSineSweep()
{
    playbackManager.setIsSweeping (false);
}

float StartupMVPAudioProcessor::getCurrSineSweepFreq()
{
    return playbackManager.getCurrSineSweepFreq();
}

void StartupMVPAudioProcessor::setReferenceVolume (float volume)
{
    playbackManager.setReferenceVolume (volume);
}

void StartupMVPAudioProcessor::setActiveCurve (juce::String curveId)
{
    activeCurveId = curveId;
}
