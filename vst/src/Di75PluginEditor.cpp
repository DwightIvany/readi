#include "Di75PluginEditor.h"

#include <cmath>

//==============================================================================
void Di75LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                       float sliderPosProportional, float rotaryStartAngle,
                                       float rotaryEndAngle, juce::Slider&)
{
    const float diameter = (float)juce::jmin(w, h) - 6.0f;
    const float radius = diameter * 0.5f;
    const float cx = (float)x + (float)w * 0.5f;
    const float cy = (float)y + (float)h * 0.5f;

    const juce::Rectangle<float> knobArea(cx - radius, cy - radius, diameter, diameter);
    const float angle = rotaryStartAngle
        + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // value arc from the start angle up to the current value
    juce::Path arc;
    arc.addCentredArc(cx, cy, radius + 3.0f, radius + 3.0f, 0.0f,
                      rotaryStartAngle, angle, true);
    g.setColour(di75TextColour());
    g.strokePath(arc, juce::PathStrokeType(2.5f));

    // metallic-blue body with a darker rim
    juce::ColourGradient gradient(juce::Colour(0x6A, 0x82, 0xE2),
                                  cx, cy - radius,
                                  juce::Colour(0x2E, 0x4A, 0xC0),
                                  cx, cy + radius, false);
    g.setGradientFill(gradient);
    g.fillEllipse(knobArea);
    g.setColour(juce::Colour(0x14, 0x2E, 0x7A));
    g.drawEllipse(knobArea, 2.0f);

    // white pointer
    const float pointerLength = radius * 0.72f;
    g.setColour(juce::Colours::white);
    g.drawLine(juce::Line<float>(cx, cy,
                                 cx + pointerLength * std::sin(angle),
                                 cy - pointerLength * std::cos(angle)),
               2.0f);
}

//==============================================================================
VuMeter::VuMeter()
{
    startTimerHz(30);
}

void VuMeter::timerCallback()
{
    ++tick;
    const float g = grv.load();

    if (g < holdGrv)
        holdGrv = g;
    if (tick % 10 == 0) // peak-hold reset ~3x per second, like the JSFX
        holdGrv = g;

    repaint();
}

void VuMeter::paint(juce::Graphics& g)
{
    const float cx = getWidth() * 0.5f;
    const float cy = 74.0f;
    const float radius = 66.0f;

    // dark inset dial
    g.setColour(juce::Colour(0x0D, 0x18, 0x40));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    g.setColour(juce::Colour(0x28, 0x36, 0x66));
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    const float start = juce::MathConstants<float>::pi * 1.25f;
    const float end = juce::MathConstants<float>::pi * 2.75f;

    auto angleFor = [&](float grDb) { return start + (grDb / 20.0f) * (end - start); };
    auto pointAt = [&](float angle, float r)
    { return juce::Point<float>(cx + r * std::sin(angle), cy - r * std::cos(angle)); };

    // tick marks; red zone across the top ~2 dB
    for (int db = 0; db <= 20; ++db)
    {
        const float a = angleFor((float)db);
        const bool major = (db % 5 == 0);
        const float r0 = radius - 3.0f;
        const float r1 = r0 - (major ? 9.0f : 5.0f);

        g.setColour(db >= 18 ? juce::Colour(0xE0, 0x3A, 0x2E)
                             : juce::Colour(0xC9, 0xD2, 0xEE));
        g.drawLine(juce::Line<float>(pointAt(a, r0), pointAt(a, r1)), major ? 2.0f : 1.0f);

        if (major)
        {
            const auto p = pointAt(a, r1 - 9.0f);
            g.setFont(juce::Font(10.0f));
            g.drawText(juce::String(db), p.x - 10.0f, p.y - 6.0f, 20.0f, 12.0f,
                       juce::Justification::centred);
        }
    }

    // hold needle (deepest recent reduction), then the live needle
    auto drawNeedle = [&](float grvValue, juce::Colour colour, float length, float thickness)
    {
        float grDb = grvValue > 0.0f ? -20.0f * std::log10(grvValue) : 20.0f;
        grDb = juce::jlimit(0.0f, 20.0f, grDb);
        g.setColour(colour);
        g.drawLine(juce::Line<float>(pointAt(angleFor(grDb), -6.0f),
                                     pointAt(angleFor(grDb), length)),
                   thickness);
    };

    drawNeedle(holdGrv, juce::Colour(0xE0, 0x3A, 0x2E), radius - 14.0f, 1.5f);
    drawNeedle(grv.load(), juce::Colours::white, radius - 14.0f, 2.5f);

    // hub
    g.setColour(juce::Colour(0x1E, 0x46, 0xAF));
    g.fillEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
    g.setColour(juce::Colour(0xC9, 0xD2, 0xEE));
    g.drawEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f, 1.0f);

    // caption
    g.setColour(di75TextColour());
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("GAIN REDUCTION dB", 0, getHeight() - 20, getWidth(), 16,
               juce::Justification::centred);
}

//==============================================================================
ValueLabel::ValueLabel(juce::AudioProcessorValueTreeState& stateToUse,
                       const juce::String& parameterId, const juce::String& namePrefix,
                       double min, double max, double snapInterval,
                       bool gain, bool highPass)
    : state(stateToUse), paramId(parameterId), prefix(namePrefix),
      minValue(min), maxValue(max), interval(snapInterval),
      gainStyle(gain), highPassStyle(highPass)
{
    setJustificationType(juce::Justification::centred);
    setFont(juce::Font(14.0f, juce::Font::bold));
    setColour(juce::Label::textColourId, di75TextColour());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setEditable(true, false);
    addListener(this);
}

juce::TextEditor* ValueLabel::createEditorComponent()
{
    auto* editor = new juce::TextEditor();
    editor->setInputRestrictions(8, "0123456789.-");
    editor->setSelectAllWhenFocused(true);
    editor->setFont(juce::Font(14.0f, juce::Font::bold));
    editor->setColour(juce::TextEditor::textColourId, juce::Colours::white);
    editor->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x14, 0x2E, 0x7A));
    editor->setColour(juce::TextEditor::highlightColourId, juce::Colour(0x4C, 0x6E, 0xE6));
    editor->setColour(juce::TextEditor::focusedOutlineColourId,
                      juce::Colours::transparentBlack);
    return editor;
}

juce::String ValueLabel::textFor(double value) const
{
    if (highPassStyle && value <= 0.0)
        return prefix + " OFF";
    if (gainStyle)
        return prefix + " " + juce::String(value, 1);
    return prefix + " " + juce::String(juce::roundToInt(value));
}

void ValueLabel::refreshFromParam()
{
    if (auto* raw = state.getRawParameterValue(paramId))
    {
        const juce::String t = textFor((double)raw->load());
        if (t != getText())
        {
            updating = true;
            setText(t, juce::dontSendNotification);
            updating = false;
        }
    }
}

void ValueLabel::labelTextChanged(juce::Label*)
{
    if (updating)
        return;

    // take the numeric tail of the label text (handles "HIGH PASS OFF" -> 0)
    const juce::String text = getText();
    int start = text.length();

    while (start > 0)
    {
        const juce::juce_wchar c = text[start - 1];
        const bool numeric = (c >= '0' && c <= '9') || c == '.' || c == '-';
        if (!numeric)
            break;
        --start;
    }

    const juce::String tail = text.substring(start).trim();
    double value = tail.isEmpty() ? 0.0 : tail.getDoubleValue();

    if (interval > 0.0)
        value = std::round(value / interval) * interval;
    value = juce::jlimit(minValue, maxValue, value);

    if (auto* param = state.getParameter(paramId))
        param->setValueNotifyingHost(
            (float)state.getParameterRange(paramId).convertTo0to1(value));
}

//==============================================================================
StateButton::StateButton(const juce::String& textOff, const juce::String& textOn)
    : juce::Button("mono-toggle"), offText(textOff), onText(textOn)
{
    setClickingTogglesState(true);
}

void StateButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown)
{
    const auto bounds = getLocalBounds().toFloat().reduced(1.5f);
    auto face = getToggleState() ? juce::Colour(0x4C, 0x6E, 0xE6)
                                 : juce::Colour(0x18, 0x34, 0x86);

    if (shouldDrawButtonAsDown)
        face = face.darker(0.25f);
    else if (shouldDrawButtonAsHighlighted)
        face = face.brighter(0.15f);

    g.setColour(face);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0x0C, 0x1C, 0x50));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    g.setColour(di75TextColour());
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(getToggleState() ? onText : offText, bounds, juce::Justification::centred);
}

//==============================================================================
namespace
{
void setupKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::Rotary);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f, true);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setVelocityBasedMode(false);
    slider.setMouseDragSensitivity(300);
}
} // namespace

Di75PluginEditor::Di75PluginEditor(Di75PluginProcessor& p)
    : juce::AudioProcessorEditor(&p),
      processor(p),
      hpLabel(p.apvts, "hp", "HIGH PASS", 0.0, 400.0, 1.0, false, true),
      threshLabel(p.apvts, "thresh", "THRESHOLD", -60.0, 0.0, 1.0, false, false),
      ratioLabel(p.apvts, "ratio", "RATIO", 4.0, 20.0, 1.0, false, false),
      atkLabel(p.apvts, "atk", "ATTACK uS", 20.0, 2000.0, 1.0, false, false),
      relLabel(p.apvts, "rel", "RELEASE mS", 20.0, 1000.0, 1.0, false, false),
      gainLabel(p.apvts, "gain", "GAIN", -20.0, 20.0, 0.1, true, false),
      monoButton("STEREO", "MONO")
{
    setLookAndFeel(&lookAndFeel);

    setupKnob(hpSlider);
    setupKnob(threshSlider);
    setupKnob(ratioSlider);
    setupKnob(atkSlider);
    setupKnob(relSlider);
    setupKnob(gainSlider);

    hpSlider.setBounds(30, 40, 80, 80);
    threshSlider.setBounds(200, 40, 80, 80);
    ratioSlider.setBounds(370, 40, 80, 80);
    atkSlider.setBounds(30, 185, 80, 80);
    relSlider.setBounds(200, 185, 80, 80);
    gainSlider.setBounds(370, 185, 80, 80);

    hpLabel.setBounds(10, 130, 120, 20);
    threshLabel.setBounds(180, 130, 120, 20);
    ratioLabel.setBounds(350, 130, 120, 20);
    atkLabel.setBounds(10, 275, 120, 20);
    relLabel.setBounds(180, 275, 120, 20);
    gainLabel.setBounds(350, 275, 120, 20);

    vuMeter.setBounds(520, 40, 250, 165);

    monoButton.setBounds(612, 212, 60, 60);

    stereoCaption.setText("Stereo", juce::dontSendNotification);
    monoCaption.setText("Mono", juce::dontSendNotification);

    for (auto* caption : { &stereoCaption, &monoCaption })
    {
        caption->setJustificationType(juce::Justification::centred);
        caption->setFont(juce::Font(12.0f));
        caption->setColour(juce::Label::textColourId, di75TextColour());
        caption->setInterceptsMouseClicks(false, false);
    }

    stereoCaption.setBounds(592, 194, 100, 16);
    monoCaption.setBounds(592, 278, 100, 16);

    addAndMakeVisible(hpSlider);
    addAndMakeVisible(threshSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(atkSlider);
    addAndMakeVisible(relSlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(hpLabel);
    addAndMakeVisible(threshLabel);
    addAndMakeVisible(ratioLabel);
    addAndMakeVisible(atkLabel);
    addAndMakeVisible(relLabel);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(vuMeter);
    addAndMakeVisible(monoButton);
    addAndMakeVisible(stereoCaption);
    addAndMakeVisible(monoCaption);

    hpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "hp", hpSlider);
    threshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "thresh", threshSlider);
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "ratio", ratioSlider);
    atkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "atk", atkSlider);
    relAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "rel", relSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "gain", gainSlider);
    monoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "mono", monoButton);

    hpLabel.refreshFromParam();
    threshLabel.refreshFromParam();
    ratioLabel.refreshFromParam();
    atkLabel.refreshFromParam();
    relLabel.refreshFromParam();
    gainLabel.refreshFromParam();

    setResizable(false, false);
    setSize(870, 300);

    startTimerHz(30);
}

Di75PluginEditor::~Di75PluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void Di75PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(di75FaceplateColour());

    g.setColour(juce::Colour(40, 40, 40));
    g.drawRect(5.0f, 5.0f, 860.0f, 290.0f, 1.0f);
}

void Di75PluginEditor::resized()
{
}

void Di75PluginEditor::timerCallback()
{
    hpLabel.refreshFromParam();
    threshLabel.refreshFromParam();
    ratioLabel.refreshFromParam();
    atkLabel.refreshFromParam();
    relLabel.refreshFromParam();
    gainLabel.refreshFromParam();

    vuMeter.setGrv(processor.grvMeter.load());
}
