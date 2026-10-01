#include "Di75PluginEditor.h"

#include <algorithm>
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
    g.setColour(juce::Colours::black);
    g.strokePath(arc, juce::PathStrokeType(2.5f));

    // white body with a black outline
    g.setColour(juce::Colour(250, 250, 250));
    g.fillEllipse(knobArea);
    g.setColour(juce::Colours::black);
    g.drawEllipse(knobArea, 2.0f);

    // black pointer
    const float pointerLength = radius * 0.72f;
    g.drawLine(juce::Line<float>(cx, cy,
                                 cx + pointerLength * std::sin(angle),
                                 cy - pointerLength * std::cos(angle)),
               2.0f);
}

//==============================================================================
// Bar geometry (local coords): bars are 25x230, input group x=24/61,
// GR x=130, output group x=199/236. The panel sits at editor (496,35), which
// puts Left_In at absolute x=520 and leaves room for scale labels at x<24.
namespace
{
constexpr float meterBarHeight = 230.0f;

float levelY(float db, float dbMin, float dbMax)
{
    return meterBarHeight * (1.0f - (db - dbMin) / (dbMax - dbMin));
}
} // namespace

MeterPanel::MeterPanel()
{
    startTimerHz(30);
}

void MeterPanel::setLevels(float newInL, float newInR, float newGrv, float newOutL, float newOutR)
{
    inL.store(newInL);
    inR.store(newInR);
    grv.store(newGrv);
    outL.store(newOutL);
    outR.store(newOutR);
}

void MeterPanel::timerCallback()
{
    ++tick;
    const float step = 15.0f / 30.0f; // ~15 dB/sec decay at 30 Hz

    // instant attack, smooth decay toward the current reading
    auto follow = [step](float target, float current)
    { return target > current ? target : std::max(target, current - step); };

    auto peakDb = [](float peak)
    {
        if (peak <= 0.0f)
            return -60.0f;
        return juce::jlimit(-60.0f, 6.0f, 20.0f * std::log10(peak));
    };

    auto reductionDb = [](float grvValue)
    {
        if (grvValue <= 0.0f)
            return 0.0f; // full-scale reduction
        return juce::jlimit(0.0f, 20.0f, -20.0f * std::log10(grvValue));
    };

    inLShown = follow(peakDb(inL.load()), inLShown);
    inRShown = follow(peakDb(inR.load()), inRShown);
    // map 0..20 dB of reduction onto the bar's -20..0 dB range
    grShown = follow(reductionDb(grv.load()) - 20.0f, grShown);
    outLShown = follow(peakDb(outL.load()), outLShown);
    outRShown = follow(peakDb(outR.load()), outRShown);

    holdInL = std::max(holdInL, inLShown);
    holdInR = std::max(holdInR, inRShown);
    holdGr = std::max(holdGr, grShown);
    holdOutL = std::max(holdOutL, outLShown);
    holdOutR = std::max(holdOutR, outRShown);

    if (tick % 10 == 0) // peak-hold reset ~3x per second, like the JSFX
    {
        holdInL = inLShown;
        holdInR = inRShown;
        holdGr = grShown;
        holdOutL = outLShown;
        holdOutR = outRShown;
    }

    repaint();
}

void MeterPanel::paint(juce::Graphics& g)
{
    // scale ticks + labels left of the input group, the GR bar, the output group
    auto drawScale = [&](float barX, std::initializer_list<float> ticks,
                         float dbMin, float dbMax)
    {
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(10.0f));
        for (float db : ticks)
        {
            const float y = levelY(db, dbMin, dbMax);
            g.fillRect(barX - 5.0f, y - 0.5f, 4.0f, 1.0f);
            g.drawText(juce::String((int)db),
                       juce::Rectangle<float>(0.0f, y - 5.0f, barX - 7.0f, 10.0f),
                       juce::Justification::centredRight);
        }
    };

    drawScale(24.0f, { -60.0f, -48.0f, -36.0f, -24.0f, -12.0f, 0.0f }, -60.0f, 6.0f);
    drawScale(130.0f, { -20.0f, -10.0f, 0.0f }, -20.0f, 0.0f);
    drawScale(199.0f, { -60.0f, -48.0f, -36.0f, -24.0f, -12.0f, 0.0f }, -60.0f, 6.0f);

    auto drawBar = [&](float barX, float shownDb, float holdDb, float dbMin, float dbMax)
    {
        const auto bar = juce::Rectangle<float>(barX, 0.0f, 25.0f, meterBarHeight);

        // grey body filled up to the current level
        const float y = levelY(juce::jlimit(dbMin, dbMax, shownDb), dbMin, dbMax);
        g.setColour(juce::Colour(150, 150, 150).withAlpha(0.8f));
        g.fillRect(bar.withTop(y));

        g.setColour(juce::Colours::black);
        g.drawRect(bar, 1.0f);

        // peak-hold marker
        const float hy = levelY(juce::jlimit(dbMin, dbMax, holdDb), dbMin, dbMax);
        g.fillRect(barX, hy - 1.0f, 25.0f, 2.0f);
    };

    drawBar(24.0f, inLShown, holdInL, -60.0f, 6.0f);
    drawBar(61.0f, inRShown, holdInR, -60.0f, 6.0f);
    drawBar(130.0f, grShown, holdGr, -20.0f, 0.0f);
    drawBar(199.0f, outLShown, holdOutL, -60.0f, 6.0f);
    drawBar(236.0f, outRShown, holdOutR, -60.0f, 6.0f);

    // captions (absolute y ~= 272)
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(13.0f));
    g.drawText("INPUT", juce::Rectangle<float>(25.0f, 237.0f, 60.0f, 16.0f),
               juce::Justification::centred);
    g.drawText("REDUCT", juce::Rectangle<float>(112.0f, 237.0f, 60.0f, 16.0f),
               juce::Justification::centred);
    g.drawText("OUTPUT", juce::Rectangle<float>(200.0f, 237.0f, 60.0f, 16.0f),
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
    setColour(juce::Label::textColourId, juce::Colours::black);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentWhite);
    setEditable(true, false);
    addListener(this);
}

juce::TextEditor* ValueLabel::createEditorComponent()
{
    auto* editor = new juce::TextEditor();
    editor->setInputRestrictions(8, "0123456789.-");
    editor->setSelectAllWhenFocused(true);
    editor->setFont(juce::Font(14.0f, juce::Font::bold));
    editor->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    editor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    editor->setColour(juce::TextEditor::highlightColourId, juce::Colour(190, 190, 190));
    editor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
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

    juce::ColourGradient gradient(juce::Colours::white, bounds.getCentreX(), bounds.getY(),
                                  juce::Colour(190, 190, 190), bounds.getCentreX(),
                                  bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 6.0f);

    if (shouldDrawButtonAsDown)
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(bounds, 6.0f);
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds, 6.0f);
    }

    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(bounds, 6.0f, 2.0f);

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

// Ring of tick dots + labels around a knob; label i of n sits at the
// value-proportional angle (min at the rotary start, max at the end).
void drawKnobRing(juce::Graphics& g, float centreX, float centreY,
                  const char* const* labels, int count)
{
    const float knobRadius = 40.0f; // 80x80 knob
    const float dotRadius = knobRadius * 1.30f;
    const float textRadius = knobRadius * 1.52f;
    const float start = juce::MathConstants<float>::pi * 1.25f;
    const float end = juce::MathConstants<float>::pi * 2.75f;

    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(12.0f));

    for (int i = 0; i < count; ++i)
    {
        const float t = count > 1 ? (float)i / (float)(count - 1) : 0.5f;
        const float a = start + t * (end - start);
        const float sx = std::sin(a);
        const float cy = std::cos(a);

        g.fillEllipse(centreX + dotRadius * sx - 1.0f,
                      centreY - dotRadius * cy - 1.0f, 2.0f, 2.0f);

        const juce::String text(labels[i]);
        if (text.isNotEmpty())
            g.drawText(text,
                       juce::Rectangle<float>(centreX + textRadius * sx - 16.0f,
                                              centreY - textRadius * cy - 6.0f,
                                              32.0f, 12.0f),
                       juce::Justification::centred);
    }
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

    meterPanel.setBounds(496, 35, 263, 262);

    monoButton.setBounds(788, 138, 60, 60);

    stereoCaption.setText("Stereo", juce::dontSendNotification);
    monoCaption.setText("Mono", juce::dontSendNotification);

    for (auto* caption : { &stereoCaption, &monoCaption })
    {
        caption->setJustificationType(juce::Justification::centred);
        caption->setFont(juce::Font(12.0f));
        caption->setColour(juce::Label::textColourId, juce::Colours::black);
        caption->setInterceptsMouseClicks(false, false);
    }

    stereoCaption.setBounds(778, 120, 80, 16);
    monoCaption.setBounds(778, 202, 80, 16);

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
    addAndMakeVisible(meterPanel);
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
    g.fillAll(juce::Colour(220, 220, 220));

    g.setColour(juce::Colours::black);
    g.drawRect(5.0f, 5.0f, 860.0f, 290.0f, 1.0f);

    // knob rings (drawn first so the knobs paint on top)
    const char* hpLabels[] = { "0", "200", "400" };
    const char* threshLabels[] = { "-60", "-50", "-40", "-30", "-20", "-10", "0" };
    const char* ratioLabels[] = { "4", "8", "12", "16", "20" };
    const char* atkLabels[] = { "20", "", "2000" };
    const char* relLabels[] = { "20", "", "1000" };
    const char* gainLabels[] = { "-20", "-10", "0", "10", "20" };

    drawKnobRing(g, 70.0f, 80.0f, hpLabels, 3);
    drawKnobRing(g, 240.0f, 80.0f, threshLabels, 7);
    drawKnobRing(g, 410.0f, 80.0f, ratioLabels, 5);
    drawKnobRing(g, 70.0f, 225.0f, atkLabels, 3);
    drawKnobRing(g, 240.0f, 225.0f, relLabels, 3);
    drawKnobRing(g, 410.0f, 225.0f, gainLabels, 5);
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

    meterPanel.setLevels(processor.inPeakL.load(), processor.inPeakR.load(),
                         processor.grvMeter.load(), processor.outPeakL.load(),
                         processor.outPeakR.load());
}
