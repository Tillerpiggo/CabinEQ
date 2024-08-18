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
                          cabinEQValueTreeManager (parameters)

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
            cabinEQValueTreeManager.initProfiles();
            
            if (! hasLoadedData)
            {
                if (listener != nullptr)
                    listener->didLoadData();
                hasLoadedData = true;
            }
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
void StartupMVPAudioProcessor::applyCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateFilterWithCurves (profile->get().getCurve (Channel::LEFT),
                                                profile->get().getCurve (Channel::RIGHT));
    }
        
}

void StartupMVPAudioProcessor::applyCurves (juce::String leftCurveName, juce::String rightCurveName)
{
//    auto leftProfile = profileNamed (leftCurveName);
//    auto rightProfile = profileNamed (rightCurveName);
//    if (leftProfile.has_value() && rightProfile.has_value())
//        playbackManager.updateFilterWithCurves (leftProfile->get().getCurve(), rightProfile->get().getCurve());
}

void StartupMVPAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
}

void StartupMVPAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

std::optional<std::reference_wrapper<Curve>> StartupMVPAudioProcessor::getCurve (juce::String profileName, Channel channel)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getCurve (channel);
    std::cout << "unable to get curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

int StartupMVPAudioProcessor::addEQNode (float frequency, float amplitude, float pan, juce::String profileName, Channel channel)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addEQNode (frequency, amplitude, pan, channel);
    return -1;
}

void StartupMVPAudioProcessor::removeEQNode (int id, juce::String profileName, Channel channel)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeEQNode (id, channel);
}

void StartupMVPAudioProcessor::updateEQNode (int id, float frequency, float amplitude, float pan, juce::String profileName, Channel channel)
{
    currProfileName = profileName; // super hacky
    std::cout << "currProfileName: " << profileName << std::endl;
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateEQNode (id, frequency, amplitude, pan, channel);
}

void StartupMVPAudioProcessor::clearEQNodes (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().resetNodes ({});
}

void StartupMVPAudioProcessor::startCalibratingEQNode (EQNode node, Channel channel)
{
    playbackManager.setCalibratingEQNode (node, channel);
    playbackManager.setIsCalibrating (true);
}

void StartupMVPAudioProcessor::updateCalibratingEQNode (EQNode node, Channel channel)
{
    playbackManager.updateCalibratingEQNode (node, channel);
}

void StartupMVPAudioProcessor::endCalibratingEQNode()
{
    playbackManager.setIsCalibrating (false);
}

float StartupMVPAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
}

void StartupMVPAudioProcessor::startTestingAt (float freq, juce::String profileName, Channel channel)
{
    auto curve = getCurve (profileName, channel);
    if (curve.has_value())
        playbackManager.startTestingFreq (freq, curve->get());
}

void StartupMVPAudioProcessor::updateTestingAt (float freq, juce::String profileName, Channel channel)
{
    auto curve = getCurve (profileName, channel);
    if (curve.has_value())
        playbackManager.updateTestingFreq (freq, curve->get());
}

void StartupMVPAudioProcessor::endTesting()
{
    playbackManager.stopTestingFreq();
}

float StartupMVPAudioProcessor::getCurrTestingFreq()
{
    return playbackManager.getCurrTestingFreq();
}

const std::vector<EQNode> StartupMVPAudioProcessor::getEQNodes (juce::String profileName, Channel channel) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getEQNodes (channel);
    return {};
}

void StartupMVPAudioProcessor::addProfile (juce::String profileName)
{
    cabinEQValueTreeManager.addProfile (profileName);
}

void StartupMVPAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEQValueTreeManager.addDuplicateProfile (profileName, oldProfileName);
}

void StartupMVPAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEQValueTreeManager.removeProfile (profileName);
}

const std::vector<juce::String> StartupMVPAudioProcessor::getProfileNames() const
{
    return cabinEQValueTreeManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEQValueTree>> StartupMVPAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
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

void StartupMVPAudioProcessor::addListener (Listener* listener)
{
    this->listener = listener;
}

void StartupMVPAudioProcessor::removeListener()
{
    this->listener = nullptr;
}

//=== Calibrate reference chords ===========================================
void StartupMVPAudioProcessor::setReferenceFreq1 (float freq)
{
    playbackManager.setReferenceFreq1 (freq);
}

void StartupMVPAudioProcessor::setReferenceFreq2 (float freq)
{
    playbackManager.setReferenceFreq2 (freq);
}

void StartupMVPAudioProcessor::setReferenceAmplLeft1 (float ampl)
{
    playbackManager.setReferenceAmplLeft1 (ampl);
}

void StartupMVPAudioProcessor::setReferenceAmplRight1 (float ampl)
{
    playbackManager.setReferenceAmplRight1 (ampl);
}

void StartupMVPAudioProcessor::setReferenceAmplLeft2 (float ampl)
{
    playbackManager.setReferenceAmplLeft2 (ampl);
}

void StartupMVPAudioProcessor::setReferenceAmplRight2 (float ampl)
{
    playbackManager.setReferenceAmplRight2 (ampl);
}

std::optional<std::reference_wrapper<CabinEQValueTree>> StartupMVPAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
}
