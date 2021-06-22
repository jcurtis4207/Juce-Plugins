/*
*   Gain Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

GainAudioProcessor::GainAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr)   // initialize parameters for this processor and no undo manager
#endif
{
    // create parameters for gain and phase
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", juce::NormalisableRange<float>{gainRangeLow, gainRangeHigh, gainRangeInterval}, 0.0f, "dB"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("phase", "Phase", true));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void GainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // before audio begins, get gain and phase values from parameters
    previousGain = *parameters.getRawParameterValue("gain");
}

void GainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    // fetch gain and phase from parameters
    float currentGain = *parameters.getRawParameterValue("gain");
    phase = *parameters.getRawParameterValue("phase");
    // if phase is flipped (false) need to multiply gain by -1
    int phaseCoefficient = (phase) ? 1 : -1;

    // clear the buffer
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // convert gain values from dBFS to 0-1 float gain values
    // multiply phase * gain for inversion
    // if gain value is changed before playing, smoothly ramp from old to new
    if (currentGain == previousGain)
    {
        buffer.applyGain(juce::Decibels::decibelsToGain(currentGain) * phaseCoefficient);
    }
    else
    {
        buffer.applyGainRamp(0, buffer.getNumSamples(), juce::Decibels::decibelsToGain(previousGain), juce::Decibels::decibelsToGain(currentGain) * phaseCoefficient);
        previousGain = currentGain;
    }
    // get buffer magnitude for meter
    bufferMagnitudeL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    bufferMagnitudeR = buffer.getMagnitude((totalNumInputChannels > 1) ? 1 : 0, 0, buffer.getNumSamples());
}

void GainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void GainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

bool GainAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool GainAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool GainAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
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

juce::AudioProcessorEditor* GainAudioProcessor::createEditor()
{
    return new GainAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainAudioProcessor();
}
