/*
*   Compressor Module
*       by Jacob Curtis
*
*   Originally inspired by Daniel Rudrich's "Simple Compressor"
* 
*   Envelope implementaiton from:
*   https://christianfloisand.wordpress.com/2014/06/09/dynamics-processing-compressorlimiter-part-1/
*
*/

#pragma once
#include <JuceHeader.h>

class Compressor
{
public:
    Compressor()
    {
        reset();
    }

    ~Compressor() {}

    void reset()
    {
        compressionLevelLeft = 0.0f;
        compressionLevelRight = 0.0f;
    }

    void updateCompressorValues(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values
        threshold = apvts.getRawParameterValue("threshold")->load();
        float attackInput = apvts.getRawParameterValue("attack")->load();
        float releaseInput = apvts.getRawParameterValue("release")->load();
        float ratio = apvts.getRawParameterValue("ratio")->load();
        makeUpGain = apvts.getRawParameterValue("makeUp")->load();
        scFreq = apvts.getRawParameterValue("scFreq")->load();
        scBypass = apvts.getRawParameterValue("scBypass")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        float mixInput = apvts.getRawParameterValue("mix")->load();
        // calculate values
        attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * (float)sampleRate));
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
        slope = 1.0f - (1.0f / ratio);
        mix = mixInput / 100.0f;
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        // initialize buffers
        dryBuffer.setSize(numChannels, maxBlockSize);
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        envelopeBuffer.setSize(numChannels, maxBlockSize);
        // initialize sidechain filters
        auto hpfCoefficients = juce::IIRCoefficients::makeHighPass(newSampleRate, scFreq);
        leftFilter.setCoefficients(hpfCoefficients);
        rightFilter.setCoefficients(hpfCoefficients);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context, double inputSampleRate)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy the buffer into the dry buffer and sidechain
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // apply sidechain filter if not bypassed
        if (!scBypass)
        {
            auto hpfCoefficients = juce::IIRCoefficients::makeHighPass(inputSampleRate, scFreq);
            leftFilter.setCoefficients(hpfCoefficients);
            rightFilter.setCoefficients(hpfCoefficients);
            float* dataLeft = sideChainBuffer.getWritePointer(0);
            float* dataRight = sideChainBuffer.getWritePointer(1);
            leftFilter.processSamples(dataLeft, bufferSize);
            rightFilter.processSamples(dataRight, bufferSize);
        }
        // create envelope
        createEnvelope(sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), envelopeBuffer.getWritePointer(0), 
            envelopeBuffer.getWritePointer(1), bufferSize, stereo);
        // apply gain reduction to sidechain
        calculateGainReduction(sideChainBuffer.getWritePointer(0), sideChainBuffer.getWritePointer(1), envelopeBuffer.getReadPointer(0), 
            envelopeBuffer.getReadPointer(1), dryBuffer.getWritePointer(0), dryBuffer.getWritePointer(1), bufferSize);
        // copy compressed signal to output
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), sideChainBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), sideChainBuffer.getReadPointer(1), bufferSize);
    }

    void createEnvelope(const float* inputLeft, const float* inputRight, float* outputLeft,
        float* outputRight, const int bufferSize, bool isStereo)
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (isStereo)
            {
                // find max of input samples
                const float maxSample = juce::jmax(std::abs(inputLeft[sample]), std::abs(inputRight[sample]));
                // apply attack
                if (compressionLevelLeft < maxSample) 
				{
                    compressionLevelLeft = maxSample + attackTime * (compressionLevelLeft - maxSample);
                }
                // apply release
                else 
				{
                    compressionLevelLeft = maxSample + releaseTime * (compressionLevelLeft - maxSample);
                }
                // write envelope
                outputLeft[sample] = compressionLevelLeft;
                outputRight[sample] = compressionLevelLeft;
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                // get absolute value of each sample
                const float sampleLeft = std::abs(inputLeft[sample]);
                const float sampleRight = std::abs(inputRight[sample]);
                // apply attack to left
                if (compressionLevelLeft < sampleLeft) 
				{
                    compressionLevelLeft = sampleLeft + attackTime * (compressionLevelLeft - sampleLeft);
                }
                // apply release to left
                else 
				{
                    compressionLevelLeft = sampleLeft + releaseTime * (compressionLevelLeft - sampleLeft);
                }
                // apply attack to right
                if (compressionLevelRight < sampleRight) 
				{
                    compressionLevelRight = sampleRight + attackTime * (compressionLevelRight - sampleRight);
                }
                // apply release to right
                else 
				{
                    compressionLevelRight = sampleRight + releaseTime * (compressionLevelRight - sampleRight);
                }
                // write envelope
                outputLeft[sample] = compressionLevelLeft;
                outputRight[sample] = compressionLevelRight;
            }
        }
    }

    void calculateGainReduction(float* inputLeft, float* inputRight, const float* envelopeLeft, 
        const float* envelopeRight, float* dryLeft, float* dryRight, const int bufferSize)
    {
        outputGainReductionLeft = 0.0f;
        outputGainReductionRight = 0.0f;
        // convert mix to sin6dB
        float dryMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (1.0 - mix)), 2.0));
        float wetMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * mix), 2.0));
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // apply threshold and ratio to envelope
            float gainReductionLeft = slope * (threshold - juce::Decibels::gainToDecibels(envelopeLeft[sample]));
            float gainReductionRight = slope * (threshold - juce::Decibels::gainToDecibels(envelopeRight[sample]));
            // remove "negative" gain reduction
            gainReductionLeft = juce::jmin(0.0f, gainReductionLeft);
            gainReductionRight = juce::jmin(0.0f, gainReductionRight);
            // set gr meter values
            if (gainReductionLeft < outputGainReductionLeft)
            {
                outputGainReductionLeft = gainReductionLeft;
            }
            if (gainReductionRight < outputGainReductionRight)
            {
                outputGainReductionRight = gainReductionRight;
            }
            // add makeup gain and convert decibels to gain values
            gainReductionLeft = std::pow(10.0f, 0.05f * (gainReductionLeft + makeUpGain));
            gainReductionRight = std::pow(10.0f, 0.05f * (gainReductionRight + makeUpGain));
            // output compressed signal back to input
            inputLeft[sample] = dryLeft[sample] * gainReductionLeft;
            inputRight[sample] = dryRight[sample] * gainReductionRight;
            // compute mix
            inputLeft[sample] *= wetMix;
            inputRight[sample] *= wetMix;
            dryLeft[sample] *= dryMix;
            dryRight[sample] *= dryMix;
            // sum wet and dry to input
            inputLeft[sample] = inputLeft[sample] + dryLeft[sample];
            inputRight[sample] = inputRight[sample] + dryRight[sample];
        }
    }

    const float getGainReductionLeft()
    {
        return (outputGainReductionLeft * -1.0f);
    }

    const float getGainReductionRight()
    {
        return (outputGainReductionRight * -1.0f);
    }

private:
    double sampleRate{ 0.0f };
    // input parameters
    float threshold{ -10.0f };
    float attackTime{ 10.0f };
    float releaseTime{ 50.0f };
    float slope{ 0.75f };
    float makeUpGain{ 0.0f };
    float scFreq{ 20.0f };
    bool scBypass{ true };
    bool stereo{ true };
    float mix{ 100.0f };
    // compression values
    float compressionLevelLeft, compressionLevelRight;
    float outputGainReductionLeft, outputGainReductionRight;
    // auxiliary buffers
    juce::AudioBuffer<float> dryBuffer, sideChainBuffer, envelopeBuffer;
    // sidechain filters
    juce::IIRFilter leftFilter, rightFilter;
};