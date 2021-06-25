/*
*   Tilt-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

TilteqAudioProcessor::TilteqAudioProcessor()
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
    // create filter parameters
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("freq", "Frequency", juce::NormalisableRange<float>{500.0f, 2000.0f, 1.0f, 0.63f}, 1000.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", juce::NormalisableRange<float>{-6.0f, 6.0f, 0.25f}, 0.0f, "dB"));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void TilteqAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // initialize dsp
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    // prepare processor chains
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    // set coefficients from parameters
    updateFilters();
}

void TilteqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // set coefficients from parameters
    updateFilters();

    // initialize dsp audio blocks
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    // process the blocks
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

void TilteqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void TilteqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

void TilteqAudioProcessor::updateFilters()
{
    float freq = parameters.getRawParameterValue("freq")->load();
    float gain = parameters.getRawParameterValue("gain")->load();
    // create low shelf with fixed 0.4Q and inverted gain
    auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), freq, 0.4f, juce::Decibels::decibelsToGain(gain * -1.0f));
    auto& leftLowShelf = leftChain.get<0>();
    auto& rightLowShelf = rightChain.get<0>();
    *leftLowShelf.coefficients = *lowShelfCoefficients;
    *rightLowShelf.coefficients = *lowShelfCoefficients;
    // create high shelf with fixed 0.4Q
    auto highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), freq, 0.4f, juce::Decibels::decibelsToGain(gain));
    auto& leftHighShelf = leftChain.get<1>();
    auto& rightHighShelf = rightChain.get<1>();
    *leftHighShelf.coefficients = *highShelfCoefficients;
    *rightHighShelf.coefficients = *highShelfCoefficients;
}

//==============================================================================
//==============================================================================
//==============================================================================

bool TilteqAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TilteqAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TilteqAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TilteqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

juce::AudioProcessorEditor* TilteqAudioProcessor::createEditor()
{
    return new TilteqAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TilteqAudioProcessor();
}
