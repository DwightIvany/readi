#pragma once

#include "Di75PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

// Faceplate blue + bisque text from the JSFX "PNG Classic" skin.
inline juce::Colour di75FaceplateColour() { return juce::Colour(0x1E, 0x46, 0xAF); }
inline juce::Colour di75TextColour() { return juce::Colour(0xF5, 0xDE, 0xB3); }

// Metallic-blue rotary knob with value arc (vector-drawn, no image assets).
class Di75LookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;
};

// Needle VU for gain reduction (1.0 = rest = far left; 0..20 dB across the sweep).
class VuMeter : public juce::Component, private juce::Timer
{
public:
    VuMeter();

    void setGrv(float newGrv) { grv.store(newGrv); }
    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    std::atomic<float> grv { 1.0f };
    float holdGrv = 1.0f;
    int tick = 0;

    JUCE_DECLARE_NON_COPYABLE(VuMeter)
};

// Editable value label: click to type a value, Enter commits, Escape cancels.
// Parsed input is snapped to the parameter interval and clamped before it is
// written back to the APVTS parameter.
class ValueLabel : public juce::Label, private juce::Label::Listener
{
public:
    ValueLabel(juce::AudioProcessorValueTreeState& stateToUse,
               const juce::String& parameterId, const juce::String& namePrefix,
               double minValue, double maxValue, double snapInterval,
               bool gainStyle, bool highPassStyle);

    void refreshFromParam();

protected:
    juce::TextEditor* createEditorComponent() override;
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

private:
    juce::String textFor(double value) const;

    juce::AudioProcessorValueTreeState& state;
    juce::String paramId, prefix;
    double minValue, maxValue, interval;
    bool gainStyle, highPassStyle;
    bool updating = false;

    JUCE_DECLARE_NON_COPYABLE(ValueLabel)
};

// Two-state toggle button that shows "STEREO" / "MONO".
class StateButton : public juce::Button
{
public:
    StateButton(const juce::String& textWhenOff, const juce::String& textWhenOn);

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;

private:
    juce::String offText, onText;

    JUCE_DECLARE_NON_COPYABLE(StateButton)
};

class Di75PluginEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Di75PluginEditor(Di75PluginProcessor&);
    ~Di75PluginEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    Di75PluginProcessor& processor;

    Di75LookAndFeel lookAndFeel;

    juce::Slider hpSlider, threshSlider, ratioSlider, atkSlider, relSlider, gainSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpAttachment,
        threshAttachment, ratioAttachment, atkAttachment, relAttachment, gainAttachment;

    ValueLabel hpLabel, threshLabel, ratioLabel, atkLabel, relLabel, gainLabel;

    VuMeter vuMeter;

    StateButton monoButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;

    juce::Label stereoCaption, monoCaption;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Di75PluginEditor)
};
