/*
*   Delay Module
*       by Jacob Curtis
*
*   Adapted from ffTapeDelay by ffAudio
*   Licensed under the BSD License
*   https://github.com/ffAudio/ffTapeDelay
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

class Delay
{
public:
    Delay(){}
    ~Delay(){}

    void setParameters(const juce::AudioProcessorValueTreeState& apvts, double inputBPM)
    {
        bpm = inputBPM;
        const float inputDelay = apvts.getRawParameterValue("delayTime")->load();
        const float inputWidth = apvts.getRawParameterValue("width")->load();
        width = static_cast<float>(inputWidth * sampleRate * 0.001f);
        feedback = apvts.getRawParameterValue("feedback")->load();
        mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        modDepth = apvts.getRawParameterValue("modDepth")->load() * 0.005f;
        modRate = apvts.getRawParameterValue("modRate")->load();
        hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        drive = apvts.getRawParameterValue("drive")->load();
        bpmSync = apvts.getRawParameterValue("bpmSync")->load();
        subdivisionIndex = static_cast<int>(apvts.getRawParameterValue("subdivisionIndex")->load());
        // convert delay time to samples based on sync status
        if (bpmSync)
        {
            delayTime = static_cast<float>(subdivisions[subdivisionIndex] * sampleRate * 60.0 / bpm);
            const float delayTimeInMilliseconds = static_cast<float>(delayTime / sampleRate * 1000.0);
            // set delayTime parameter to millisecond value of subdivision
            apvts.getParameter("delayTime")->beginChangeGesture();
            apvts.getParameter("delayTime")->setValueNotifyingHost(juce::NormalisableRange<float>(
                1.0f, 2000.0f, 1.0f).convertTo0to1(delayTimeInMilliseconds));
            apvts.getParameter("delayTime")->endChangeGesture();
        }
        else
        {
            delayTime = static_cast<float>(inputDelay * sampleRate * 0.001f);
        }
        // ensure delay time doesn't exceed delayBufferSize
        delayTime = juce::jmin(delayTime, static_cast<float>(delayBufferSize - bufferSize));
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        // initialize buffers
        dryBuffer.setSize(numOutputs, maxBlockSize);
        wetBuffer.setSize(numOutputs, maxBlockSize);
        wetBuffer.clear();
        delayBufferSize = static_cast<int>(2.0 * (bufferSize + sampleRate));
        delayBuffer.setSize(numOutputs, delayBufferSize);
        delayBuffer.clear();
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        modChain.prepare(spec);
        filterChain.prepare(spec);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // copy input to dry buffer
        for (int channel = 0; channel < numOutputs; channel++)
        {
            dryBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
        }
        // create delay in wet buffer
        fillDelayBuffer();
        readDelayBuffer();
        applyFilters();
        applyDistortion();
        applyModulation();
        applyFeedback();
        // increment write position and wrap around delay buffer length
        writePosition = (writePosition + bufferSize) % delayBufferSize;
        // mix wet and dry buffers to wet buffer and copy to output
        applyMix();
        for (int channel = 0; channel < numOutputs; channel++)
        {
            inputBuffer.copyFrom(channel, 0, wetBuffer.getReadPointer(channel), bufferSize);
        }
    }

    // write dry buffer into delay buffer
    void fillDelayBuffer()
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            if (bufferSize + writePosition <= delayBufferSize)
            {
                delayBuffer.copyFrom(channel, writePosition, dryBuffer.getReadPointer(channel), bufferSize);
            }
            else
            {
                const int bufferRemaining = delayBufferSize - writePosition;
                delayBuffer.copyFrom(channel, writePosition, dryBuffer.getReadPointer(channel), bufferRemaining);
                delayBuffer.copyFrom(channel, 0, dryBuffer.getReadPointer(channel, bufferRemaining), bufferSize - bufferRemaining);
            }
        }
    }

    // write delay buffer with delay into wet buffer
    void readDelayBuffer()
    {
        std::array<int, numOutputs> readPosition = {
            static_cast<int>(delayBufferSize + writePosition - delayTime - width) % delayBufferSize,
            static_cast<int>(delayBufferSize + writePosition - delayTime + width) % delayBufferSize
        };
        for (int channel = 0; channel < numOutputs; channel++)
        {
            if (bufferSize + readPosition[channel] <= delayBufferSize)
            {
                wetBuffer.copyFrom(channel, 0, delayBuffer.getReadPointer(channel, readPosition[channel]), bufferSize);
            }
            else
            {
                const int bufferRemaining = delayBufferSize - readPosition[channel];
                wetBuffer.copyFrom(channel, 0, delayBuffer.getReadPointer(channel, readPosition[channel]), bufferRemaining);
                wetBuffer.copyFrom(channel, bufferRemaining, delayBuffer.getReadPointer(channel), bufferSize - bufferRemaining);
            }
        }
    }

    // add feedback from wet buffer to delay buffer
    void applyFeedback()
    {
        const float feedbackGain = feedback * 0.01f;
        for (int channel = 0; channel < numOutputs; channel++)
        {
            if (delayBufferSize > bufferSize + writePosition)
            {
                delayBuffer.addFromWithRamp(channel, writePosition, 
                    wetBuffer.getWritePointer(channel), bufferSize, feedbackGain, feedbackGain);
            }
            else
            {
                const int bufferRemaining = delayBufferSize - writePosition;
                delayBuffer.addFromWithRamp(channel, bufferRemaining, 
                    wetBuffer.getWritePointer(channel), bufferRemaining, feedbackGain, feedbackGain);
                delayBuffer.addFromWithRamp(channel, 0, wetBuffer.getWritePointer(channel), 
                    bufferSize - bufferRemaining, feedbackGain, feedbackGain);
            }
        }
    } 

    // apply filters to wet buffer
    void applyFilters()
    {
        *filterChain.get<0>().state = *juce::dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2)[0];
        *filterChain.get<1>().state = *juce::dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2)[0];
        juce::dsp::AudioBlock<float> filterBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

    // apply atan waveshaping to wet buffer
    void applyDistortion()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                float wetSample = wetBuffer.getSample(channel, sample);
                // apply gain, distortion, and autogain
                wetSample *= (drive / 30.0f) + 1.0f;
                wetSample = (2.0f / juce::float_Pi) * atan((juce::float_Pi / 2.0f) * wetSample);    
                wetSample *= juce::Decibels::decibelsToGain(drive / -12.0f);
                wetBuffer.setSample(channel, sample, wetSample);
            }
        }
    }

    // apply chorus to wet buffer
    void applyModulation()
    {
        modChain.setCentreDelay(1.0f);
        modChain.setFeedback(0.0f);
        modChain.setMix(1.0f);
        modChain.setDepth(modDepth);
        modChain.setRate(modRate);
        juce::dsp::AudioBlock<float> modBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> modContext(modBlock);
        modChain.process(modContext);
    }

    // mix dry and wet buffers to wet buffer
    void applyMix()
    {
        // sin3dB
        const float dryMix = std::sin(0.5f * juce::float_Pi * (1.0f - mix));
        const float wetMix = std::sin(0.5f * juce::float_Pi * mix);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                const float wetSample = wetBuffer.getSample(channel, sample) * wetMix;
                wetBuffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    int delayBufferSize{ 0 };
    int writePosition{ 0 };
    double bpm{ 0.0 };
    float delayTime{ 0.0f };
    float feedback{ 0.0f };
    float width{ 0.0f };
    float mix{ 0.0f };
    float modRate{ 0.0f };
    float modDepth{ 0.0f };
    float hpfFreq{ 0.0f };
    float lpfFreq{ 0.0f };
    float drive{ 0.0f };
    bool bpmSync{ false };
    int subdivisionIndex{ 0 };
    const std::array<float, 13> subdivisions{ 0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, 
        (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f };
    juce::AudioBuffer<float> dryBuffer, wetBuffer, delayBuffer;
    juce::dsp::Chorus<float> modChain;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
        juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};