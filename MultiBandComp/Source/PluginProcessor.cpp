/*
*   MultiBandComp Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

MultiBandCompAudioProcessor::MultiBandCompAudioProcessor()
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
    // filter parameters
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("crossoverFreqB", 
        "Crossover Frequency 1", juce::NormalisableRange<float>(20.0f, 15000.0f, 1.0f, 0.25f), 200.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("crossoverFreqA", 
        "Crossover Frequency 2", juce::NormalisableRange<float>(20.0f, 15000.0f, 1.0f, 0.25f), 1000.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("crossoverFreqC", 
        "Crossover Frequency 3", juce::NormalisableRange<float>(20.0f, 15000.0f, 1.0f, 0.25f), 5000.0f, "Hz"));
    // compression parameters
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("stereo", "Stereo Mode", true));
    for (int band = 1; band <= numBands; band++)
    {
        const auto bandNum = juce::String(band);
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("threshold" + bandNum, 
            "Band " + bandNum + " Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f, "dB"));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("attack" + bandNum, 
            "Band " + bandNum + " Attack", juce::NormalisableRange<float>(0.5f, 100.0f, 0.5f), 10.0f, "ms"));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("release" + bandNum, 
            "Band " + bandNum + " Release", juce::NormalisableRange<float>(1.0f, 1100.0f, 1.0f), 50.0f, "ms"));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("ratio" + bandNum, 
            "Band " + bandNum + " Ratio", juce::NormalisableRange<float>(1.0f, 16.0f, 1.0f), 4.0f, " : 1"));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("makeUp" + bandNum, 
            "Band " + bandNum + " Make Up", juce::NormalisableRange<float>(-10.0f, 20.0f, 0.1f), 0.0f, "dB"));
    }
    parameters.state = juce::ValueTree("savedParams");
}

void MultiBandCompAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    multibandComp.prepare(sampleRate, samplesPerBlock);
    multibandComp.setParameters(parameters, listen);
}

void MultiBandCompAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    multibandComp.setParameters(parameters, listen);
    multibandComp.process(buffer);
    gainReduction = multibandComp.getGainReduction();
}

void MultiBandCompAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void MultiBandCompAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

bool MultiBandCompAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MultiBandCompAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MultiBandCompAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiBandCompAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

juce::AudioProcessorEditor* MultiBandCompAudioProcessor::createEditor()
{
    return new MultiBandCompAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBandCompAudioProcessor();
}
