/*
*   De-esser Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

DeesserAudioProcessor::DeesserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr)
#endif
{
    // create de-esser parameters
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("threshold", 
        "Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.5f), 0.0f, "dB"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("crossoverFreq", 
        "Frequency", juce::NormalisableRange<float>(200.0f, 15000.0f, 1.0f, 0.25f), 4000.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("attack", 
        "Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.35f), 0.1f, "ms"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("release", 
        "Release", juce::NormalisableRange<float>(5.0f, 100.0f, 0.1f, 0.35f), 10.0f, "ms"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("stereo", "Stereo", true));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("wide", "Wide Band", false));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void DeesserAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    deesser.prepare(sampleRate, samplesPerBlock);
    deesser.setParameters(parameters, listen);
}

void DeesserAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // apply de-esser
    deesser.setParameters(parameters, listen);
    deesser.process(buffer);
    // get gain reduction for meter
    gainReductionLeft = deesser.getGainReductionLeft();
    gainReductionRight = deesser.getGainReductionRight();
}

void DeesserAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void DeesserAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // create xml from binary
    std::unique_ptr<juce::XmlElement> inputXml(getXmlFromBinary(data, sizeInBytes));
    // check that inputXml returned correctly
    if (inputXml != nullptr)
    {
        // if inputXml tag name matches tree state tag name
        if (inputXml->hasTagName(parameters.state.getType()))
        {
            // copy xml into tree state
            parameters.state = juce::ValueTree::fromXml(*inputXml);
        }
    }
}

//==============================================================================
//==============================================================================
//==============================================================================

bool DeesserAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DeesserAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DeesserAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DeesserAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

juce::AudioProcessorEditor* DeesserAudioProcessor::createEditor()
{
    return new DeesserAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DeesserAudioProcessor();
}
