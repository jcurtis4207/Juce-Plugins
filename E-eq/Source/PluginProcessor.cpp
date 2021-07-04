/*
*   E-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

EeqAudioProcessor::EeqAudioProcessor()
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
    // create parameters for the highpass filter
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("hpfFreq", "HPF Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>("hpfSlope", "HPF Slope", filterSlopes, 0));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("hpfBypass", "HPF Bypass", false));
    // create parameters for the lowpass filter
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("lpfFreq", "LPF Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f, "Hz"));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>("lpfSlope", "LPF Slope", filterSlopes, 0));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("lpfBypass", "LPF Bypass", false));
    // create parameters for peak bands
    for (int i = 1; i <= 4; i++)
    {
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Freq", "Band " + juce::String(i) + " Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), defaultFreq[i - 1], "Hz"));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Gain", "Band " + juce::String(i) + " Gain", juce::NormalisableRange<float>(-20.0f, 20.0f, 0.25f), 0.0f, "dB"));  
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Q", "Band " + juce::String(i) + " Q", juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f, 0.4f), 1.0f, "Q"));
        // create shelf/bell switch for bands 1 and 4
        if (i == 1 || i == 4)
        {
            parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("band" + juce::String(i) + "Bell", "Band " + juce::String(i) + " Bell", false));
        }
    }
    // set state to an empty value tree
    parameters.state = juce::ValueTree("savedParams");
}

void EeqAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // initialize dsp
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    // prepare processor chains
    processChain.prepare(spec);
    // set coefficients from parameters
    setCoefficients(parameters);
}

void EeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // set coefficients from parameters
    setCoefficients(parameters);
    // initialize dsp audio block
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    // process the block
    processChain.process(context);
}

void EeqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // create xml with state information
    std::unique_ptr <juce::XmlElement> outputXml(parameters.state.createXml());
    // save xml to binary
    copyXmlToBinary(*outputXml, destData);
}

void EeqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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

// populate settings struct from parameters
ParameterValues getParameterValues(juce::AudioProcessorValueTreeState& apvts)
{
    ParameterValues newValues;
    // get all paramter values for filters
    newValues.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
    newValues.hpfSlope = (int)apvts.getRawParameterValue("hpfSlope")->load();
    newValues.hpfBypass = apvts.getRawParameterValue("hpfBypass")->load();
    newValues.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
    newValues.lpfSlope = (int)apvts.getRawParameterValue("lpfSlope")->load();
    newValues.lpfBypass = apvts.getRawParameterValue("lpfBypass")->load();
    // get all parameter values for parametric bands
    newValues.band1Freq = apvts.getRawParameterValue("band1Freq")->load();
    newValues.band1Gain = apvts.getRawParameterValue("band1Gain")->load();
    newValues.band1Q = apvts.getRawParameterValue("band1Q")->load();
    newValues.band2Freq = apvts.getRawParameterValue("band2Freq")->load();
    newValues.band2Gain = apvts.getRawParameterValue("band2Gain")->load();
    newValues.band2Q = apvts.getRawParameterValue("band2Q")->load();
    newValues.band3Freq = apvts.getRawParameterValue("band3Freq")->load();
    newValues.band3Gain = apvts.getRawParameterValue("band3Gain")->load();
    newValues.band3Q = apvts.getRawParameterValue("band3Q")->load();
    newValues.band4Freq = apvts.getRawParameterValue("band4Freq")->load();
    newValues.band4Gain = apvts.getRawParameterValue("band4Gain")->load();
    newValues.band4Q = apvts.getRawParameterValue("band4Q")->load();
    newValues.band1Bell = apvts.getRawParameterValue("band1Bell")->load();
    newValues.band4Bell = apvts.getRawParameterValue("band4Bell")->load();
    return newValues;
}

// update parametric bands coefficients from settings struct
void EeqAudioProcessor::updateEqBands(const ParameterValues& inputValues)
{
    // create peak coefficients for all bands
    auto band1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), inputValues.band1Freq, inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain));
    auto band2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), inputValues.band2Freq, inputValues.band2Q, juce::Decibels::decibelsToGain(inputValues.band2Gain));
    auto band3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), inputValues.band3Freq, inputValues.band3Q, juce::Decibels::decibelsToGain(inputValues.band3Gain));
    auto band4Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), inputValues.band4Freq, inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain));
    // create shelf coefficients if needed
    if (!inputValues.band1Bell)
    {
        band1Coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), inputValues.band1Freq, inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain));
    }
    if (!inputValues.band4Bell)
    {
        band4Coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), inputValues.band4Freq, inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain));
    }
    // apply coefficients to process chain
    *processChain.get<ChainIndex::Band1>().state = *band1Coefficients;
    *processChain.get<ChainIndex::Band2>().state = *band2Coefficients;
    *processChain.get<ChainIndex::Band3>().state = *band3Coefficients;
    *processChain.get<ChainIndex::Band4>().state = *band4Coefficients;
}

// update filter coefficients from settings struct
void EeqAudioProcessor::updateFilters(const ParameterValues& inputValues)
{
    // bypass hpf
    if (inputValues.hpfBypass)
    {
        processChain.setBypassed<ChainIndex::HPF>(true);
    }
    // update hpf coefficients
    else
    {
        processChain.setBypassed<ChainIndex::HPF>(false);
        auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(inputValues.hpfFreq, getSampleRate(), 2 * (inputValues.hpfSlope + 1));
        *processChain.get<ChainIndex::HPF>().state = *hpfCoefficients[0];
    }
    // bypass lpf
    if (inputValues.lpfBypass)
    {
        processChain.setBypassed<ChainIndex::LPF>(true);
    }
    // update lpf coefficients
    else
    {
        processChain.setBypassed<ChainIndex::LPF>(false);
        auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(inputValues.lpfFreq, getSampleRate(), 2 * (inputValues.lpfSlope + 1));
        *processChain.get<ChainIndex::LPF>().state = *lpfCoefficients[0];
    }
}

// update settings struct and start updating bands
void EeqAudioProcessor::setCoefficients(juce::AudioProcessorValueTreeState& apvts)
{
    // get parameters
    auto parameterValues = getParameterValues(apvts);
    // set coefficients from parameters
    updateFilters(parameterValues);
    updateEqBands(parameterValues);
}

//==============================================================================
//==============================================================================
//==============================================================================

bool EeqAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EeqAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool EeqAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EeqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

juce::AudioProcessorEditor* EeqAudioProcessor::createEditor()
{
     return new EeqAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EeqAudioProcessor();
}
