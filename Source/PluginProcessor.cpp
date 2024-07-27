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
                       ), parameters (*this, nullptr, "Params", createParameterLayout (EQNodeManager::NUM_PTS)),
                          eqNodeManager (parameters)

#endif
{
    //parameters.state = juce::ValueTree("savedParams");
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
    playbackManager.setSampleRate (sampleRate);
    
    this->sampleRate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;
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
    
    
    std::cout << "Saving state as: " << state.getType().toString() << std::endl;
    const auto& clearEQTree = state.getChildWithName (juce::Identifier ("ClearEQ"));
    if (clearEQTree.isValid())
    {
        std::cout << "ClearEQValueTree (numNodes: " << clearEQTree.getNumChildren() << ")" << std::endl;
        juce::Identifier idId ("id");
        juce::Identifier idFrequency ("frequency");
        juce::Identifier idAmplitude ("amplitude");
        if (clearEQTree.getNumChildren() > 0)
        {
            for (const auto& eqNode : clearEQTree)
            {
                float id = eqNode.getProperty (idId);
                float freq = eqNode.getProperty (idFrequency);
                float ampl = eqNode.getProperty (idAmplitude);
                
                std::cout << "EQNode (id: " << id << ", freq: " << freq << ", ampl: " << ampl << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "NO_TREE" << std::endl;
    }
    std::cout << xml->toString() << std::endl;
    std::cout << std::endl;
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
            std::cout << "Getting state as: " << parameters.state.getType().toString() << std::endl;
            const auto& clearEQTree = parameters.state.getChildWithName (juce::Identifier ("ClearEQ"));
            if (clearEQTree.isValid())
            {
                std::cout << "ClearEQValueTree (numNodes: " << clearEQTree.getNumChildren() << ")" << std::endl;
                if (clearEQTree.getNumChildren() > 0)
                {
                    juce::Identifier idId ("id");
                    juce::Identifier idFrequency ("frequency");
                    juce::Identifier idAmplitude ("amplitude");
                    for (const auto& eqNode : clearEQTree)
                    {
                        float id = eqNode.getProperty (idId);
                        float freq = eqNode.getProperty (idFrequency);
                        float ampl = eqNode.getProperty (idAmplitude);
                        
                        std::cout << "EQNode (id: " << id << ", freq: " << freq << ", ampl: " << ampl << ")" << std::endl;
                    }
                }
            }
            else
            {
                std::cout << "NO_TREE" << std::endl;
            }
            std::cout << xmlState->toString() << std::endl;
            std::cout << std::endl;
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StartupMVPAudioProcessor();
}



juce::AudioProcessorValueTreeState::ParameterLayout StartupMVPAudioProcessor::createParameterLayout(int numPoints)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
//
//    // Define range for each parameter
//    float defaultVal = 0.f; // for everything
//    juce::NormalisableRange<float> gainRange (-24.0f, 48.0, 0.05f, 1.0f);
//    juce::NormalisableRange<float> panRange (-24.0f, 24.0, 0.05f, 1.0f);
//    juce::NormalisableRange<float> phaseRange (-3.14, 3.14, 0.01f, 1.0f);
//    //
//
//    // Add numPoints gain parameters
//    for ( int i = 0; i < numPoints; i++ ) {
//        juce::String paramID = "gain_" + std::to_string(i);
//
//        layout.add (std::make_unique<juce::AudioParameterFloat>(
//            juce::ParameterID(paramID, 1),
//            paramID,
//            gainRange,
//            defaultVal
//        ));
//    }
//    
//    // Add numPoints balance parameters to match
//    for ( int i = 0; i < numPoints; i++ ) {
//        juce::String paramID = "pan_" + std::to_string(i);
//
//        layout.add(std::make_unique<juce::AudioParameterFloat>(
//            juce::ParameterID(paramID, 1),
//            paramID,
//            panRange,
//            defaultVal
//        ));
//    }
//    
//    // Add numPoints phase parameters as well
//    for ( int i = 0; i < numPoints; i++ ) {
//        juce::String paramID = "phase_" + std::to_string(i);
//
//        layout.add(std::make_unique<juce::AudioParameterFloat>(
//            juce::ParameterID(paramID, 1),
//            paramID,
//            phaseRange,
//            defaultVal
//        ));
//    }

    return {};
}

//==============================================================================
void StartupMVPAudioProcessor::applyCurve()
{
//    if (!hasPreparedFilter)
//    {
//        spec.sampleRate = sampleRate;
//        spec.maximumBlockSize = samplesPerBlock;
//        spec.numChannels = getTotalNumInputChannels();
//        playbackManager.prepare (spec);
//    }
    
    for (int i = 0; i < EQNodeManager::NUM_PTS; ++i)
    {
        std::string idx = std::to_string (i);
        
        double gain = parameters.getRawParameterValue ("gain_" + idx)->load();
        double pan = parameters.getRawParameterValue ("pan_" + idx)->load();
        
//        playbackManager.setAmplitudeAtIdx (i, gain);
//        playbackManager.setPanAtIdx (i, pan);
    }
    
    playbackManager.updateWithCurve (eqNodeManager.getCurve());
}

void StartupMVPAudioProcessor::setIsBypassed (bool isBypassed)
{
    playbackManager.setIsBypassed (isBypassed);
}

void StartupMVPAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

const Curve& StartupMVPAudioProcessor::getCurve() const
{
    return eqNodeManager.getCurve();
}

void StartupMVPAudioProcessor::addEQNode (float frequency, float amplitude, float pan)
{
    eqNodeManager.addEQNode (frequency, amplitude, pan);
}

void StartupMVPAudioProcessor::removeEQNode (int id)
{
    eqNodeManager.removeEQNode (id);
}

void StartupMVPAudioProcessor::updateEQNode (int id, float frequency, float amplitude, float pan)
{
    eqNodeManager.updateEQNode (id, frequency, amplitude, pan);
}

const std::vector<EQNode>& StartupMVPAudioProcessor::getEQNodes() const
{
    return eqNodeManager.getNodes();
}
