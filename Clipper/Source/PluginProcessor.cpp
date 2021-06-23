/*
*   Clipper Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ClipperAudioProcessor::ClipperAudioProcessor()
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
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f, "dB"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("ceiling", "Ceiling", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f, "dB"));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void ClipperAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // initialize dsp
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    processChain.prepare(spec);
    // setup clipper
    clipper.prepare(sampleRate, 2, samplesPerBlock);
}

void ClipperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // set clipper values from parameters
    updateClipperValues(parameters);
    // apply dsp
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    clipper.process(context, getSampleRate());
    // get gain reduction for meter
    gainReductionLeft = clipper.getGainReductionLeft();
    gainReductionRight = clipper.getGainReductionRight();
    // apply dsp gain chain
    processChain.process(context);
}

void ClipperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void ClipperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

void ClipperAudioProcessor::updateClipperValues(juce::AudioProcessorValueTreeState& apvts)
{
    // get parameter values
    float threshold = apvts.getRawParameterValue("threshold")->load();
    float ceiling = apvts.getRawParameterValue("ceiling")->load();
    // set threshold in clipper module
    clipper.setThreshold(threshold);
    // apply autogain based on threshold
    processChain.get<0>().setGainDecibels(threshold * -1.0f);
    // apply ceiling gain
    processChain.get<1>().setGainDecibels(ceiling);
}

//==============================================================================
//==============================================================================
//==============================================================================

bool ClipperAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ClipperAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ClipperAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

juce::AudioProcessorEditor* ClipperAudioProcessor::createEditor()
{
    return new ClipperAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipperAudioProcessor();
}
