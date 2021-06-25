/*
*   Compressor Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Compressored on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

CompressorAudioProcessorEditor::CompressorAudioProcessorEditor(CompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), meter(audioProcessor.gainReductionLeft, audioProcessor.gainReductionRight)
{
    // threshold
    thresholdSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    thresholdSlider.setTextValueSuffix(" dB");
    thresholdSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(thresholdSlider);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold", thresholdSlider);
    thresholdLabel.setText("Threshold", juce::NotificationType::dontSendNotification);
    thresholdLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(thresholdLabel);
    // attack
    attackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    attackSlider.setTextValueSuffix(" ms");
    attackSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(attackSlider);
    attackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack", attackSlider);
    attackLabel.setText("Attack", juce::NotificationType::dontSendNotification);
    attackLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(attackLabel);
    // release
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    releaseSlider.setTextValueSuffix(" ms");
    releaseSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(releaseSlider);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseSlider);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    releaseLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(releaseLabel);
    // ratio
    ratioSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    ratioSlider.setTextValueSuffix(" : 1");
    ratioSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(ratioSlider);
    ratioAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ratio", ratioSlider);
    ratioLabel.setText("Ratio", juce::NotificationType::dontSendNotification);
    ratioLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(ratioLabel);
    // makeUp
    makeUpSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    makeUpSlider.setTextValueSuffix(" dB");
    makeUpSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(makeUpSlider);
    makeUpAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "makeUp", makeUpSlider);
    makeUpLabel.setText("Make Up", juce::NotificationType::dontSendNotification);
    makeUpLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(makeUpLabel);
    // sc freq
    scFreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    scFreqSlider.setTextValueSuffix(" Hz");
    scFreqSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(scFreqSlider);
    scFreqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "scFreq", scFreqSlider);
    scFreqLabel.setText("SC Freq", juce::NotificationType::dontSendNotification);
    scFreqLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(scFreqLabel);
    // sc bypass
    scBypassButton.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(scBypassButton);
    scBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "scBypass", scBypassButton);
    scBypassLabel.setText("SC Filter", juce::NotificationType::dontSendNotification);
    scBypassLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(scBypassLabel);
    // stereo
    stereoButton.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(stereoButton);
    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stereo", stereoButton);
    stereoLabel.setText("Link Mode", juce::NotificationType::dontSendNotification);
    stereoLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(stereoLabel);
    // mix
    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixSlider.setTextValueSuffix(" %");
    mixSlider.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(mixSlider);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixSlider);
    mixLabel.setText("Mix", juce::NotificationType::dontSendNotification);
    mixLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(mixLabel);
    // gr meter
    addAndMakeVisible(meter);
    grLabel.setText("GR", juce::NotificationType::dontSendNotification);
    grLabel.setLookAndFeel(&compressorLookAndFeel);
    addAndMakeVisible(grLabel);

    setSize(250, 400);
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void CompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw powerlines
    powerLine.drawPowerLine(g, 125.0f, 10.0f, 100.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 110.0f, 30.0f, 4, 0, "Compressor");
}

void CompressorAudioProcessorEditor::resized()
{
    int col1XPosition = 20;
    int col2XPosition = 90;
    int col3XPosition = 160;
    int sliderWidth = 50;
    // col 1
    thresholdSlider.setBounds(col1XPosition, 60, sliderWidth, sliderWidth);
    thresholdLabel.setBounds(thresholdSlider.getX(), thresholdSlider.getBottom(), sliderWidth, 20);
    attackSlider.setBounds(col1XPosition, 160, sliderWidth, sliderWidth);
    attackLabel.setBounds(attackSlider.getX(), attackSlider.getBottom(), sliderWidth, 20);
    makeUpSlider.setBounds(col1XPosition, 260, sliderWidth, sliderWidth);
    makeUpLabel.setBounds(makeUpSlider.getX(), makeUpSlider.getBottom(), sliderWidth, 20);
    stereoButton.setBounds(col1XPosition, 340, sliderWidth, 20);
    stereoLabel.setBounds(stereoButton.getX(), stereoButton.getBottom(), sliderWidth, 20);
    // col 2
    ratioSlider.setBounds(col2XPosition, 60, sliderWidth, sliderWidth);
    ratioLabel.setBounds(ratioSlider.getX(), ratioSlider.getBottom(), sliderWidth, 20);
    releaseSlider.setBounds(col2XPosition, 160, sliderWidth, sliderWidth);
    releaseLabel.setBounds(releaseSlider.getX(), releaseSlider.getBottom(), sliderWidth, 20);
    scFreqSlider.setBounds(col2XPosition, 260, sliderWidth, sliderWidth);
    scFreqLabel.setBounds(scFreqSlider.getX(), scFreqSlider.getBottom(), sliderWidth, 20);
    scBypassButton.setBounds(col2XPosition, 340, sliderWidth, 20);
    scBypassLabel.setBounds(scBypassButton.getX(), scBypassButton.getBottom(), sliderWidth, 20);
    // col 3
    grLabel.setBounds(col3XPosition + 5, 55, 44, 20);
    meter.setBounds(col3XPosition, 70, meter.getMeterWidth(), meter.getMeterHeight());
    mixSlider.setBounds(col3XPosition + 4, 325, sliderWidth, sliderWidth);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getBottom(), sliderWidth, 20);
}