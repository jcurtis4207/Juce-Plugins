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

using namespace juce;

struct Parameters {
    float delayTime, feedback, width, mix, 
        modRate, modDepth, hpfFreq, lpfFreq, drive;
    bool bpmSync;
    int subdivisionIndex;
};

class Delay
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts, double inputBPM)
    {
        bpm = inputBPM;
        const float inputDelay = apvts.getRawParameterValue("delayTime")->load();
        const float inputWidth = apvts.getRawParameterValue("width")->load();
        parameters.width = static_cast<float>(inputWidth * sampleRate * 0.001f);
        parameters.feedback = apvts.getRawParameterValue("feedback")->load();
        parameters.mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        parameters.modDepth = apvts.getRawParameterValue("modDepth")->load() * 0.005f;
        parameters.modRate = apvts.getRawParameterValue("modRate")->load();
        parameters.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        parameters.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        parameters.drive = apvts.getRawParameterValue("drive")->load();
        parameters.bpmSync = apvts.getRawParameterValue("bpmSync")->load();
        parameters.subdivisionIndex = static_cast<int>(
            apvts.getRawParameterValue("subdivisionIndex")->load());
        setDelayTime(apvts, inputDelay);
        // ensure delay time doesn't exceed delayBufferSize
        parameters.delayTime = jmin(parameters.delayTime, 
            static_cast<float>(delayBufferSize - bufferSize));
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        dryBuffer.setSize(numOutputs, maxBlockSize);
        wetBuffer.setSize(numOutputs, maxBlockSize);
        wetBuffer.clear();
        delayBufferSize = static_cast<int>(2.0 * (bufferSize + sampleRate));
        delayBuffer.setSize(numOutputs, delayBufferSize);
        delayBuffer.clear();
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        modChain.prepare(spec);
        filterChain.prepare(spec);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        dryBuffer.makeCopyOf(inputBuffer, true);
        fillDelayBuffer();
        readDelayBuffer();
        applyFilters();
        applyDistortion();
        applyModulation();
        applyFeedback();
        incrementWritePosition();
        mixToOutput(inputBuffer);
    }

private:
    void incrementWritePosition()
    {
        writePosition = (writePosition + bufferSize) % delayBufferSize;
    }

    void setDelayTime(const AudioProcessorValueTreeState& apvts, float inputDelay)
    {
        // convert delay time to samples based on sync status
        if (parameters.bpmSync)
        {
            parameters.delayTime = static_cast<float>(
                subdivisions[parameters.subdivisionIndex] * sampleRate * 60.0 / bpm);
            const float delayTimeInMilliseconds = static_cast<float>(
                parameters.delayTime / sampleRate * 1000.0);
            // set delayTime parameter to millisecond value of subdivision
            apvts.getParameter("delayTime")->beginChangeGesture();
            apvts.getParameter("delayTime")->setValueNotifyingHost(NormalisableRange<float>(
                1.0f, 2000.0f, 1.0f).convertTo0to1(delayTimeInMilliseconds));
            apvts.getParameter("delayTime")->endChangeGesture();
        }
        else
        {
            parameters.delayTime = static_cast<float>(inputDelay * sampleRate * 0.001f);
        }
    }

    // write dry buffer into delay buffer
    void fillDelayBuffer()
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            if (bufferSize + writePosition <= delayBufferSize)
            {
                delayBuffer.copyFrom(channel, writePosition, 
                    dryBuffer.getReadPointer(channel), bufferSize);
            }
            else
            {
                const int bufferRemaining = delayBufferSize - writePosition;
                delayBuffer.copyFrom(channel, writePosition, 
                    dryBuffer.getReadPointer(channel), bufferRemaining);
                delayBuffer.copyFrom(channel, 0, dryBuffer.getReadPointer(channel, bufferRemaining), 
                    bufferSize - bufferRemaining);
            }
        }
    }

    // write delay buffer with delay into wet buffer
    void readDelayBuffer()
    {
        std::array<int, numOutputs> readPosition = {
            static_cast<int>(delayBufferSize + writePosition - 
                parameters.delayTime - parameters.width) % delayBufferSize,
            static_cast<int>(delayBufferSize + writePosition - 
                parameters.delayTime + parameters.width) % delayBufferSize
        };
        for (int channel = 0; channel < numOutputs; channel++)
        {
            if (bufferSize + readPosition[channel] <= delayBufferSize)
            {
                wetBuffer.copyFrom(channel, 0, 
                    delayBuffer.getReadPointer(channel, readPosition[channel]), bufferSize);
            }
            else
            {
                const int bufferRemaining = delayBufferSize - readPosition[channel];
                wetBuffer.copyFrom(channel, 0, 
                    delayBuffer.getReadPointer(channel, readPosition[channel]), bufferRemaining);
                wetBuffer.copyFrom(channel, bufferRemaining, 
                    delayBuffer.getReadPointer(channel), bufferSize - bufferRemaining);
            }
        }
    }

    // add feedback from wet buffer to delay buffer
    void applyFeedback()
    {
        const float feedbackGain = parameters.feedback * 0.01f;
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

    void applyFilters()
    {
        *filterChain.get<0>().state = *dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(parameters.hpfFreq, sampleRate, 2)[0];
        *filterChain.get<1>().state = *dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(parameters.lpfFreq, sampleRate, 2)[0];
        dsp::AudioBlock<float> filterBlock(wetBuffer);
        dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

    void applyDistortion()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                float wetSample = wetBuffer.getSample(channel, sample);
                wetSample *= (parameters.drive / 30.0f) + 1.0f;                         // drive
                wetSample = (2.0f / float_Pi) * atan((float_Pi / 2.0f) * wetSample);    // atan waveshaping
                wetSample *= Decibels::decibelsToGain(parameters.drive / -12.0f);       // autogain
                wetBuffer.setSample(channel, sample, wetSample);
            }
        }
    }

    void applyModulation()
    {
        modChain.setCentreDelay(1.0f);
        modChain.setFeedback(0.0f);
        modChain.setMix(1.0f);
        modChain.setDepth(parameters.modDepth);
        modChain.setRate(parameters.modRate);
        dsp::AudioBlock<float> modBlock(wetBuffer);
        dsp::ProcessContextReplacing<float> modContext(modBlock);
        modChain.process(modContext);
    }

    void mixToOutput(AudioBuffer<float>& buffer)
    {
        const float dryMix = std::sin(0.5f * float_Pi * (1.0f - parameters.mix));
        const float wetMix = std::sin(0.5f * float_Pi * parameters.mix);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                const float wetSample = wetBuffer.getSample(channel, sample) * wetMix;
                buffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    int delayBufferSize{ 0 };
    int writePosition{ 0 };
    double bpm{ 0.0 };
    Parameters parameters;
    const std::array<float, 13> subdivisions{ 0.25f, (0.5f/3.0f), 0.375f, 0.5f, 
        (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f };
    AudioBuffer<float> dryBuffer, wetBuffer, delayBuffer;
    dsp::Chorus<float> modChain;
    using StereoFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, 
        dsp::IIR::Coefficients<float>>;
    dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};