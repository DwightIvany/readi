#pragma once

#include "Di75Dsp.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <vector>

class Di75PluginEditor;

class Di75PluginProcessor : public juce::AudioProcessor
{
public:
    Di75PluginProcessor();
    ~Di75PluginProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // meter feeds, written on the audio thread, read by the editor timer
    std::atomic<float> grvMeter { 1.0f };
    std::atomic<float> inPeakL { 0.0f };
    std::atomic<float> inPeakR { 0.0f };
    std::atomic<float> outPeakL { 0.0f };
    std::atomic<float> outPeakR { 0.0f };

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float rawParamValue(const char* paramId) const;

    Di75Dsp dsp;
    std::vector<float> interleaveScratch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Di75PluginProcessor)
};
