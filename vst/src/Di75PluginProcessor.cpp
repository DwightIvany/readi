// DSP derived from Stillwell 1175 (BSD); original source:
// https://github.com/stillwellaudio/jsfx/blob/master/1175
// Full license text retained in src/Di75Dsp.cpp.

#include "Di75PluginProcessor.h"
#include "Di75PluginEditor.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
std::unique_ptr<juce::AudioParameterFloat> makeFloat(const juce::String& id, const juce::String& name,
                                                     float minValue, float maxValue, float interval,
                                                     float defaultValue)
{
    return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { id, 1 }, name,
                                                       juce::NormalisableRange<float>(minValue, maxValue, interval),
                                                       defaultValue);
}
} // namespace

Di75PluginProcessor::Di75PluginProcessor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Di75PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(makeFloat("hp", "High Pass", 0.0f, 400.0f, 1.0f, 80.0f));
    layout.add(makeFloat("thresh", "Threshold", -60.0f, 0.0f, 1.0f, -12.0f));
    layout.add(makeFloat("ratio", "Ratio", 4.0f, 20.0f, 1.0f, 4.0f));
    layout.add(makeFloat("gain", "Gain", -20.0f, 20.0f, 0.1f, 0.0f));
    layout.add(makeFloat("atk", "Attack", 20.0f, 2000.0f, 1.0f, 20.0f));
    layout.add(makeFloat("rel", "Release", 20.0f, 1000.0f, 1.0f, 250.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "mono", 1 },
                                                          "Mono", false));

    return layout;
}

float Di75PluginProcessor::rawParamValue(const char* paramId) const
{
    if (auto* value = apvts.getRawParameterValue(paramId))
        return value->load();
    return 0.0f;
}

void Di75PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    dsp.setSampleRate(sampleRate);
    dsp.reset();
    grvMeter = 1.0f;
}

bool Di75PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void Di75PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int inChannels = getTotalNumInputChannels();
    const int outChannels = getTotalNumOutputChannels();
    const int numFrames = buffer.getNumSamples();

    for (int ch = 2; ch < outChannels; ++ch)
        buffer.clear(ch, 0, numFrames);

    auto blockPeak = [](const float* data, int n)
    {
        float peak = 0.0f;
        for (int i = 0; i < n; ++i)
            peak = std::max(peak, std::abs(data[i]));
        return peak;
    };

    // input peaks must be measured before the buffer is overwritten
    inPeakL = (inChannels > 0) ? blockPeak(buffer.getReadPointer(0), numFrames) : 0.0f;
    inPeakR = (inChannels > 1) ? blockPeak(buffer.getReadPointer(1), numFrames) : 0.0f;

    dsp.setParams(rawParamValue("hp"), rawParamValue("thresh"), rawParamValue("ratio"),
                  rawParamValue("gain"), rawParamValue("atk"), rawParamValue("rel"),
                  rawParamValue("mono") > 0.5f);

    if (inChannels >= 2 && outChannels >= 2 && numFrames > 0)
    {
        if ((int)interleaveScratch.size() < 2 * numFrames)
            interleaveScratch.resize(2 * numFrames);

        const float* inL = buffer.getReadPointer(0);
        const float* inR = buffer.getReadPointer(1);
        float* scratch = interleaveScratch.data();

        for (int i = 0; i < numFrames; ++i)
        {
            scratch[2 * i] = inL[i];
            scratch[2 * i + 1] = inR[i];
        }

        dsp.process(scratch, numFrames);

        float* outL = buffer.getWritePointer(0);
        float* outR = buffer.getWritePointer(1);

        for (int i = 0; i < numFrames; ++i)
        {
            outL[i] = scratch[2 * i];
            outR[i] = scratch[2 * i + 1];
        }
    }
    else if (outChannels >= 2)
    {
        // stereo in/out is the only supported layout; never leave garbage behind
        buffer.clear(0, 0, numFrames);
        buffer.clear(1, 0, numFrames);
    }

    outPeakL = (outChannels > 0) ? blockPeak(buffer.getReadPointer(0), numFrames) : 0.0f;
    outPeakR = (outChannels > 1) ? blockPeak(buffer.getReadPointer(1), numFrames) : 0.0f;
    grvMeter = (float)dsp.currentGrv();
}

juce::AudioProcessorEditor* Di75PluginProcessor::createEditor()
{
    return new Di75PluginEditor(*this);
}

void Di75PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void Di75PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Di75PluginProcessor();
}
