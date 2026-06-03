#include "PluginEditor.h"

namespace
{
    const juce::Colour kBg      { 0xff1a1614 };
    const juce::Colour kPanel   { 0xff272220 };
    const juce::Colour kAccent  { 0xffd98c3a }; // warm amber
    const juce::Colour kText    { 0xffe8ddd0 };
}

//==============================================================================
DecapLookAndFeel::DecapLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, kText);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, kPanel);
    setColour (juce::ComboBox::textColourId, kText);
    setColour (juce::ComboBox::outlineColourId, kAccent.withAlpha (0.4f));
    setColour (juce::ComboBox::arrowColourId, kAccent);
    setColour (juce::PopupMenu::backgroundColourId, kPanel);
    setColour (juce::PopupMenu::textColourId, kText);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kAccent);
    setColour (juce::TextButton::buttonColourId, kPanel);
    setColour (juce::TextButton::textColourOnId, kAccent);
    setColour (juce::TextButton::textColourOffId, kText);
    setColour (juce::ToggleButton::textColourId, kText);
}

void DecapLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width,
                                         int height, float sliderPos,
                                         float startAngle, float endAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (8.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + sliderPos * (endAngle - startAngle);

    // Drop shadow.
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (bounds.translated (0.0f, 2.5f));

    // Tick ring.
    const int ticks = 11;
    for (int t = 0; t < ticks; ++t)
    {
        const float ta = startAngle + (float) t / (ticks - 1) * (endAngle - startAngle);
        const float r1 = radius + 2.0f, r2 = radius + 6.0f;
        const juce::Point<float> p1 (centre.x + std::sin (ta) * r1, centre.y - std::cos (ta) * r1);
        const juce::Point<float> p2 (centre.x + std::sin (ta) * r2, centre.y - std::cos (ta) * r2);
        g.setColour (kText.withAlpha (ta <= angle ? 0.9f : 0.25f));
        g.drawLine ({ p1, p2 }, 1.4f);
    }

    // Brushed-metal body (radial gradient).
    juce::ColourGradient grad (kPanel.brighter (0.25f), centre.x, bounds.getY(),
                               kPanel.darker (0.4f),    centre.x, bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillEllipse (bounds);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (bounds, 1.5f);

    // Inset cap.
    auto cap = bounds.reduced (radius * 0.34f);
    g.setColour (kPanel.darker (0.2f));
    g.fillEllipse (cap);

    // Amber value arc.
    juce::Path arc;
    const float arcR = radius + 4.0f;
    arc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (kAccent);
    g.strokePath (arc, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // Pointer.
    juce::Path pointer;
    const float pl = radius * 0.78f;
    pointer.addRoundedRectangle (-1.6f, -pl, 3.2f, pl * 0.5f, 1.6f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (kAccent.brighter (0.3f));
    g.fillPath (pointer);
}

void DecapLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                         bool isHighlighted, bool /*isDown*/)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = b.getToggleState();

    g.setColour (on ? kAccent.withAlpha (0.9f) : kPanel.brighter (isHighlighted ? 0.15f : 0.0f));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (on ? kAccent.brighter (0.2f) : kAccent.withAlpha (0.35f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.2f);

    g.setColour (on ? juce::Colours::black.withAlpha (0.85f) : kText);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (b.getButtonText(), bounds, juce::Justification::centred);
}

//==============================================================================
DecapitoneAudioProcessorEditor::DecapitoneAudioProcessorEditor (DecapitoneAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    setUpKnob (driveKnob,   "drive",   "DRIVE");
    setUpKnob (toneKnob,    "tone",    "TONE");
    setUpKnob (lowCutKnob,  "lowcut",  "LOW CUT");
    setUpKnob (highCutKnob, "highcut", "HIGH CUT");
    setUpKnob (mixKnob,     "mix",     "MIX");
    setUpKnob (outputKnob,  "output",  "OUTPUT");

    modelBox.addItemList ({ "A - Tape", "E - EMI", "N - Neve", "T - Triode", "P - Pentode" }, 1);
    modelBox.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (modelBox);
    modelAttach = std::make_unique<APVTS::ComboBoxAttachment> (proc.apvts, "model", modelBox);

    for (auto* tb : { &punishButton, &steepButton, &thumpButton })
    {
        tb->setClickingTogglesState (true);
        addAndMakeVisible (*tb);
    }
    punishAttach = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, "punish", punishButton);
    steepAttach  = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, "steep",  steepButton);
    thumpAttach  = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, "thump",  thumpButton);

    // Preset selector, backed by host programs.
    addAndMakeVisible (presetBox);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setJustificationType (juce::Justification::centred);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0 && idx != proc.getCurrentProgram())
            proc.setCurrentProgram (idx);
    };

    addAndMakeVisible (prevPreset);
    addAndMakeVisible (nextPreset);
    prevPreset.onClick = [this]
    {
        const int n = proc.getNumPrograms();
        proc.setCurrentProgram ((proc.getCurrentProgram() - 1 + n) % n);
        refreshPresetBox();
    };
    nextPreset.onClick = [this]
    {
        const int n = proc.getNumPrograms();
        proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % n);
        refreshPresetBox();
    };
    refreshPresetBox();

    setResizable (true, true);
    if (auto* c = getConstrainer())
    {
        c->setFixedAspectRatio (kDesignW / kDesignH);
        setResizeLimits (480, 336, 1200, 840);
    }
    setSize ((int) kDesignW, (int) kDesignH);
    startTimerHz (30);
}

DecapitoneAudioProcessorEditor::~DecapitoneAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DecapitoneAudioProcessorEditor::refreshPresetBox()
{
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
}

void DecapitoneAudioProcessorEditor::setUpKnob (LabeledKnob& k, const juce::String& id,
                                                const juce::String& text)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
    addAndMakeVisible (k.slider);

    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setColour (juce::Label::textColourId, kText);
    k.label.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (k.label);

    k.attach = std::make_unique<APVTS::SliderAttachment> (proc.apvts, id, k.slider);
}

//==============================================================================
void DecapitoneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Scale all drawing so the UI looks identical at any window size.
    const float s = (float) getWidth() / kDesignW;
    g.fillAll (kBg);
    g.addTransform (juce::AffineTransform::scale (s));

    auto full = juce::Rectangle<float> (0, 0, kDesignW, kDesignH);

    // Header bar.
    auto header = full.removeFromTop (56);
    g.setColour (kPanel);
    g.fillRect (header);
    g.setColour (kAccent);
    g.setFont (juce::Font (28.0f, juce::Font::bold));
    g.drawText ("DECAPITONE", header.reduced (18, 0).removeFromLeft (280),
                juce::Justification::centredLeft);
    g.setColour (kText.withAlpha (0.55f));
    g.setFont (juce::Font (12.0f));
    g.drawText ("analog saturation", header.withTrimmedRight (150).reduced (18, 0),
                juce::Justification::centredRight);

    // Output meter (right edge).
    auto meterArea = juce::Rectangle<float> (kDesignW - 22, 70, 12, kDesignH - 90);
    g.setColour (kPanel);
    g.fillRoundedRectangle (meterArea, 3.0f);
    const float db = juce::Decibels::gainToDecibels (meterLevel, -60.0f);
    const float norm = juce::jmap (db, -60.0f, 6.0f, 0.0f, 1.0f);
    auto fill = meterArea.withTop (meterArea.getBottom() - meterArea.getHeight() * juce::jlimit (0.0f, 1.0f, norm));
    g.setColour (db > 0.0f ? juce::Colours::red : kAccent);
    g.fillRoundedRectangle (fill, 3.0f);
}

void DecapitoneAudioProcessorEditor::resized()
{
    // Work in design coordinates, then scale the whole component tree.
    const float s = (float) getWidth() / kDesignW;

    juce::Rectangle<int> area (0, 0, (int) kDesignW, (int) kDesignH);
    area.reduce (16, 16);
    area.removeFromTop (40);          // header
    area.removeFromRight (20);        // meter gutter

    // Preset row.
    auto presetRow = area.removeFromTop (28);
    prevPreset.setBounds (presetRow.removeFromLeft (28));
    presetRow.removeFromLeft (4);
    presetBox.setBounds (presetRow.removeFromLeft (200));
    presetRow.removeFromLeft (4);
    nextPreset.setBounds (presetRow.removeFromLeft (28));

    area.removeFromTop (10);

    // Style + mode toggles row.
    auto top = area.removeFromTop (34);
    modelBox.setBounds (top.removeFromLeft (180).reduced (0, 3));
    top.removeFromLeft (12);
    auto toggle = [&top] (juce::ToggleButton& b) { b.setBounds (top.removeFromLeft (84).reduced (2, 3)); top.removeFromLeft (6); };
    toggle (punishButton);
    toggle (steepButton);
    toggle (thumpButton);

    area.removeFromTop (14);

    // Two rows of three knobs.
    auto knobCell = [] (juce::Rectangle<int> r, LabeledKnob& k)
    {
        k.label.setBounds (r.removeFromTop (16));
        k.slider.setBounds (r);
    };

    const int knobW = area.getWidth() / 3;
    auto row1 = area.removeFromTop (area.getHeight() / 2);
    auto row2 = area;

    knobCell (row1.removeFromLeft (knobW).reduced (6), driveKnob);
    knobCell (row1.removeFromLeft (knobW).reduced (6), toneKnob);
    knobCell (row1.removeFromLeft (knobW).reduced (6), mixKnob);

    knobCell (row2.removeFromLeft (knobW).reduced (6), lowCutKnob);
    knobCell (row2.removeFromLeft (knobW).reduced (6), highCutKnob);
    knobCell (row2.removeFromLeft (knobW).reduced (6), outputKnob);

    // Apply the global scale so everything tracks the window size.
    const auto t = juce::AffineTransform::scale (s);
    for (auto* c : getChildren())
        c->setTransform (t);
}

void DecapitoneAudioProcessorEditor::timerCallback()
{
    const float target = proc.outputLevel.load();
    meterLevel = target > meterLevel ? target : meterLevel * 0.85f + target * 0.15f;
    // The preset box can drift out of sync if the host changes program; keep it honest.
    if (presetBox.getSelectedId() - 1 != proc.getCurrentProgram())
        refreshPresetBox();
    repaint();
}
