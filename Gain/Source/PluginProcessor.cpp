/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAudioProcessor::GainAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr)   // initialize parameters for this processor and no undo manager
#endif
{
    // create parameter for gain
    juce::NormalisableRange<float> gainRange(GAIN_RANGE_LOW, GAIN_RANGE_HIGH, GAIN_RANGE_INTERVAL);
    parameters.createAndAddParameter(std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(GAIN_ID, GAIN_NAME, GAIN_NAME, gainRange, 0.5f, nullptr, nullptr));
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

GainAudioProcessor::~GainAudioProcessor()
{
}

void GainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // before audio begins, get gain value from parameters
    previousGain =*parameters.getRawParameterValue(GAIN_ID);

}

void GainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    // fetch gain from parameters
    float currentGain = *parameters.getRawParameterValue(GAIN_ID);
    // clear the buffer
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // if gain value is changed before playing, smoothly ramp from old to new
    if (currentGain == previousGain)
    {
        buffer.applyGain(juce::Decibels::decibelsToGain(currentGain));
    }
    else
    {   // convert gain values from dBFS to 0-1 float gain values
        buffer.applyGainRamp(0, buffer.getNumSamples(), juce::Decibels::decibelsToGain(previousGain), juce::Decibels::decibelsToGain(currentGain));
        previousGain = currentGain;
    }

    // old way to manually changed sample gain based on slider value
    /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) 
        {
            channelData[sample] = channelData[sample] * juce::Decibels::decibelsToGain(gain);
        }
    }*/
}

void GainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void GainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

//==============================================================================
//==============================================================================
//==============================================================================

const juce::String GainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

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

double GainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GainAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String GainAudioProcessor::getProgramName(int index)
{
    return {};
}

void GainAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}
void GainAudioProcessor::releaseResources()
{
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

bool GainAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GainAudioProcessor::createEditor()
{
    return new GainAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainAudioProcessor();
}
