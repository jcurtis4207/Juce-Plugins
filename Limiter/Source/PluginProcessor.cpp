/*
*   Limiter Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

LimiterAudioProcessor::LimiterAudioProcessor()
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
    // create limiter parameters
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>{-40.0f, 0.0f, 0.1f}, 0.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>{0.1f, 200.0f, 0.1f, 0.35f}, 1.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("ceiling", "Ceiling", juce::NormalisableRange<float>{-40.0f, 0.0f, 0.1f}, 0.0f));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void LimiterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // initialize dsp
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    // prepare processor chains
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    // update limiter
    updateLimiterValues(parameters);
}

void LimiterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // update limiter
    updateLimiterValues(parameters);
    // initialize dsp audio blocks
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    // process the blocks
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    // get buffer magnitudes for meter
    bufferMagnitudeL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    bufferMagnitudeR = buffer.getMagnitude((totalNumInputChannels > 1) ? 1 : 0, 0, buffer.getNumSamples());
}

void LimiterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void LimiterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // create xml from binary
    std::unique_ptr<juce::XmlElement> inputXml(getXmlFromBinary(data, sizeInBytes));
    // check that theParams returned correctly
    if (inputXml != nullptr)
    {
        // if theParams tag name matches tree state tag name
        if (inputXml->hasTagName(parameters.state.getType()))
        {
            // copy xml into tree state
            parameters.state = juce::ValueTree::fromXml(*inputXml);
        }
    }
}

void LimiterAudioProcessor::updateLimiterValues(juce::AudioProcessorValueTreeState& apvts)
{
    // get values from parameters
    threshold = apvts.getRawParameterValue("threshold")->load();
    release = apvts.getRawParameterValue("release")->load();
    ceiling = apvts.getRawParameterValue("ceiling")->load();
    // update left chain
    leftChain.get<ChainIndex::Limiter>().setThreshold(threshold);
    leftChain.get<ChainIndex::Limiter>().setRelease(release);
    leftChain.get<ChainIndex::Gain>().setGainDecibels(ceiling);
    // update right chain
    rightChain.get<ChainIndex::Limiter>().setThreshold(threshold);
    rightChain.get<ChainIndex::Limiter>().setRelease(release);
    rightChain.get<ChainIndex::Gain>().setGainDecibels(ceiling);
}

//==============================================================================
//==============================================================================
//==============================================================================

bool LimiterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool LimiterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool LimiterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LimiterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

juce::AudioProcessorEditor* LimiterAudioProcessor::createEditor()
{
    return new LimiterAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LimiterAudioProcessor();
}