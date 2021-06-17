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
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("hpfFreq", "HPF Frequency", juce::NormalisableRange<float>{20.0f, 20000.0f, 1.0f, 0.25f}, 20.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>("hpfSlope", "HPF Slope", filterSlopes, 0));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("hpfBypass", "HPF Bypass", false));
    // create parameters for the lowpass filter
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("lpfFreq", "LPF Frequency", juce::NormalisableRange<float>{20.0f, 20000.0f, 1.0f, 0.25f}, 20000.0f));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>("lpfSlope", "LPF Slope", filterSlopes, 0));
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("lpfBypass", "LPF Bypass", false));
    // create parameters for the peak filters
    for (int i = 1; i <= 4; i++)
    {
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Freq", "Band " + juce::String(i) + " Frequency", juce::NormalisableRange<float>{20.0f, 20000.0f, 1.0f, 0.25f}, defaultFreq[i-1]));
        parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Gain", "Band " + juce::String(i) + " Gain", juce::NormalisableRange<float>{-20.0f, 20.0f, 0.25f}, 0.0f));
        // bands 2 and 3 have Q parameters
        if (i == 2 || i == 3)
        {
            parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("band" + juce::String(i) + "Q", "Band " + juce::String(i) + " Q", juce::NormalisableRange<float>{0.1f, 10.0f, 0.1f, 0.4f}, 1.0f));
        }
        // bands 1 and 4 have shelf parameters
        else
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
    spec.numChannels = 1;

    // prepare processor chains
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    // set coefficients from parameters
    setCoefficients(parameters);
}

void EeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // set coefficients from parameters
    setCoefficients(parameters);

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
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& parameters)
{
    ChainSettings settings;
    // get all paramter values for filters
    settings.hpfFreq = parameters.getRawParameterValue("hpfFreq")->load();
    settings.hpfSlope = (int)parameters.getRawParameterValue("hpfSlope")->load();
    settings.hpfBypass = parameters.getRawParameterValue("hpfBypass")->load();
    settings.lpfFreq = parameters.getRawParameterValue("lpfFreq")->load();
    settings.lpfSlope = (int)parameters.getRawParameterValue("lpfSlope")->load();
    settings.lpfBypass = parameters.getRawParameterValue("lpfBypass")->load();
    // get all parameter values for parametric bands
    settings.band1Freq = parameters.getRawParameterValue("band1Freq")->load();
    settings.band1Gain = parameters.getRawParameterValue("band1Gain")->load();
    settings.band1Bell = parameters.getRawParameterValue("band1Bell")->load();
    settings.band2Freq = parameters.getRawParameterValue("band2Freq")->load();
    settings.band2Gain = parameters.getRawParameterValue("band2Gain")->load();
    settings.band2Q = parameters.getRawParameterValue("band2Q")->load();
    settings.band3Freq = parameters.getRawParameterValue("band3Freq")->load();
    settings.band3Gain = parameters.getRawParameterValue("band3Gain")->load();
    settings.band3Q = parameters.getRawParameterValue("band3Q")->load();
    settings.band4Freq = parameters.getRawParameterValue("band4Freq")->load();
    settings.band4Gain = parameters.getRawParameterValue("band4Gain")->load();
    settings.band4Bell = parameters.getRawParameterValue("band4Bell")->load();
    return settings;
}

// update parametric bands coefficients from settings struct
void EeqAudioProcessor::updateEqBands(const ChainSettings& chainSettings)
{
    // band 1 as shelf
    if (!chainSettings.band1Bell)
    {
        auto band1Coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), chainSettings.band1Freq, 1.0, juce::Decibels::decibelsToGain(chainSettings.band1Gain));
        *leftChain.get<ChainIndex::Band1>().coefficients = *band1Coefficients;
        *rightChain.get<ChainIndex::Band1>().coefficients = *band1Coefficients;
    }
    // band 1 as peak
    else
    {
        auto band1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.band1Freq, 1.0, juce::Decibels::decibelsToGain(chainSettings.band1Gain));
        *leftChain.get<ChainIndex::Band1>().coefficients = *band1Coefficients;
        *rightChain.get<ChainIndex::Band1>().coefficients = *band1Coefficients;
    }
    // band 2
    auto band2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.band2Freq, chainSettings.band2Q, juce::Decibels::decibelsToGain(chainSettings.band2Gain));
    *leftChain.get<ChainIndex::Band2>().coefficients = *band2Coefficients;
    *rightChain.get<ChainIndex::Band2>().coefficients = *band2Coefficients;
    // band 3
    auto band3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.band3Freq, chainSettings.band3Q, juce::Decibels::decibelsToGain(chainSettings.band3Gain));
    *leftChain.get<ChainIndex::Band3>().coefficients = *band3Coefficients;
    *rightChain.get<ChainIndex::Band3>().coefficients = *band3Coefficients;
    // band 4 as shelf
    if (!chainSettings.band4Bell)
    {
        auto band4Coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), chainSettings.band4Freq, 1.0, juce::Decibels::decibelsToGain(chainSettings.band4Gain));
        *leftChain.get<ChainIndex::Band4>().coefficients = *band4Coefficients;
        *rightChain.get<ChainIndex::Band4>().coefficients = *band4Coefficients;
    }
    // band 4 as peak
    else
    {
        auto band4Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.band4Freq, 1.0, juce::Decibels::decibelsToGain(chainSettings.band4Gain));
        *leftChain.get<ChainIndex::Band4>().coefficients = *band4Coefficients;
        *rightChain.get<ChainIndex::Band4>().coefficients = *band4Coefficients;
    }
}

// update filter coefficients from settings struct
void EeqAudioProcessor::updateFilters(const ChainSettings& chainSettings)
{
    // bypass hpf
    if (chainSettings.hpfBypass)
    {
        leftChain.setBypassed<ChainIndex::HPF>(true);
        rightChain.setBypassed<ChainIndex::HPF>(true);
    }
    // update hpf coefficients
    else
    {
        leftChain.setBypassed<ChainIndex::HPF>(false);
        rightChain.setBypassed<ChainIndex::HPF>(false);
        auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.hpfFreq, getSampleRate(), 2 * (chainSettings.hpfSlope + 1));
        auto& leftHPF = leftChain.get<ChainIndex::HPF>();
        auto& rightHPF = rightChain.get<ChainIndex::HPF>();
        *leftHPF.coefficients = *hpfCoefficients[0];
        *rightHPF.coefficients = *hpfCoefficients[0];
    }
    // bypass lpf
    if (chainSettings.lpfBypass)
    {
        leftChain.setBypassed<ChainIndex::LPF>(true);
        rightChain.setBypassed<ChainIndex::LPF>(true);
    }
    // update lpf coefficients
    else
    {
        leftChain.setBypassed<ChainIndex::LPF>(false);
        rightChain.setBypassed<ChainIndex::LPF>(false);
        auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.lpfFreq, getSampleRate(), 2 * (chainSettings.lpfSlope + 1));
        auto& leftLPF = leftChain.get<ChainIndex::LPF>();
        auto& rightLPF = rightChain.get<ChainIndex::LPF>();
        *leftLPF.coefficients = *lpfCoefficients[0];
        *rightLPF.coefficients = *lpfCoefficients[0];
    }
}

// update settings struct and start updating bands
void EeqAudioProcessor::setCoefficients(juce::AudioProcessorValueTreeState& apvts)
{
    // get parameters
    auto chainSettings = getChainSettings(apvts);
    // set coefficients from parameters
    updateFilters(chainSettings);
    updateEqBands(chainSettings);
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
