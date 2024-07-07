/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
                       ), parameters (*this, nullptr, "Parameters", createParameterLayout (10))

#endif
{
    parameters.state = juce::ValueTree("savedParams");
    isCalibrating = false;
    isBypassed = false;
    gainProcessor.setGainDecibels(0.f);
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
    calibrationManager.setSampleRate (sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    gainFilter.prepare (spec);
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
    
    // Get channel pointers and clear buffer
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    if (isCalibrating)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const std::pair<float, float> value = calibrationManager.getNextSample();
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
        
        if (isBypassed)
        {
            gainFilter.process (context);
        }
        else
        {
            gainProcessor.process (context);
        }
    }
    
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
    
//    std::unique_ptr <juce::XmlElement> xml (parameters.state.createXml());
//    copyXmlToBinary(*xml, destData);
}

void StartupMVPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
//    std::unique_ptr <juce::XmlElement> savedParams(getXmlFromBinary(data, sizeInBytes));
//    if (savedParams != nullptr)
//    {
//        if (savedParams->hasTagName(parameters.state.getType()))
//        {
//            parameters.state = juce::ValueTree::fromXml(*savedParams);
//            
//            for (int i = 0; i < SetPointManager::NUM_SET_POINTS; ++i)
//            {
//                std::string idx = std::to_string (i);
//                
//                double gain = parameters.getRawParameterValue("gain_" + idx)->load();
//                double pan = parameters.getRawParameterValue("pan_" + idx)->load();
//                double phase = parameters.getRawParameterValue ("phase_" + idx)->load();
////                calibrationManager.setGainAtIdx (i, gain);
////                calibrationManager.setPanAtIdx (i, pan);
////                calibrationManager.setPhaseAtIdx (i, phase);
//            }
//            
//            gainFilter.update (calibrationManager.getCurve(), FFT_SIZE);
////            balanceFilter.update (calibrationManager.getBalanceCurve(), FFT_SIZE);
//        }
//    }
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

    // Define range for each parameter
    float defaultVal = 0.f; // for everything
    juce::NormalisableRange<float> gainRange (-24.0f, 48.0, 0.05f, 1.0f);
    juce::NormalisableRange<float> panRange (-24.0f, 24.0, 0.05f, 1.0f);
    juce::NormalisableRange<float> phaseRange (-3.14, 3.14, 0.01f, 1.0f);
    //

    // Add numPoints gain parameters
    for ( int i = 0; i < numPoints; i++ ) {
        juce::String paramID = "gain_" + std::to_string(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramID, 1),
            paramID,
            gainRange,
            defaultVal
        ));
    }
    
    // Add numPoints balance parameters to match
    for ( int i = 0; i < numPoints; i++ ) {
        juce::String paramID = "pan_" + std::to_string(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramID, 1),
            paramID,
            panRange,
            defaultVal
        ));
    }
    
    // Add numPoints phase parameters as well
    for ( int i = 0; i < numPoints; i++ ) {
        juce::String paramID = "phase_" + std::to_string(i);

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramID, 1),
            paramID,
            phaseRange,
            defaultVal
        ));
    }

    return layout;
}

//==============================================================================
const Curve& StartupMVPAudioProcessor::getCurve() const
{
    return calibrationManager.getCurve();
}

void StartupMVPAudioProcessor::applyCurve()
{
    gainFilter.update (calibrationManager.getCurve(), FFT_SIZE);
}

void StartupMVPAudioProcessor::toggleBypass()
{
    isBypassed = ! isBypassed;
}

void StartupMVPAudioProcessor::toggleCalibration()
{
    isCalibrating = ! isCalibrating;
}

void StartupMVPAudioProcessor::calibrateWith (CalibrationChoice choice)
{
    calibrationManager.calibrateWith (choice);
}

const Question& StartupMVPAudioProcessor::getCurrentQuestion() const
{
    return calibrationManager.getCurrentQuestion();
}

bool StartupMVPAudioProcessor::isPlayingFirstNote() const
{
    return calibrationManager.isPlayingFirstNote();
}

void StartupMVPAudioProcessor::changeReferencePanTo (float newReferencePan)
{
    calibrationManager.changeReferencePanTo (newReferencePan);
}
