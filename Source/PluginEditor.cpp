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
    auto area = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float dim   = juce::jmin (area.getWidth(), area.getHeight());
    auto bounds       = area.withSizeKeepingCentre (dim, dim).reduced (dim * 0.16f);
    const auto radius = bounds.getWidth() * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + sliderPos * (endAngle - startAngle);

    // Soft drop shadow.
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.12f));
        g.fillEllipse (bounds.translated (0.0f, (float) i).expanded ((float) i * 0.6f));
    }

    // Background track arc (full sweep, dim).
    const float trackR = radius + dim * 0.10f;
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, trackR, trackR, 0.0f, startAngle, endAngle, true);
    g.setColour (kPanel.brighter (0.10f));
    g.strokePath (track, juce::PathStrokeType (dim * 0.045f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Active value arc with a subtle glow.
    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, trackR, trackR, 0.0f, startAngle, angle, true);
    g.setColour (kAccent.withAlpha (0.25f));
    g.strokePath (arc, juce::PathStrokeType (dim * 0.085f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    g.setColour (kAccent);
    g.strokePath (arc, juce::PathStrokeType (dim * 0.045f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // Knob body: vertical metal gradient + rim.
    juce::ColourGradient grad (kPanel.brighter (0.32f), centre.x, bounds.getY(),
                               kPanel.darker (0.45f),    centre.x, bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillEllipse (bounds);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawEllipse (bounds, 1.4f);
    g.setColour (kText.withAlpha (0.06f));            // top highlight rim
    g.drawEllipse (bounds.reduced (1.4f).removeFromTop (radius), 1.2f);

    // Inset cap.
    auto cap = bounds.reduced (radius * 0.40f);
    juce::ColourGradient capGrad (kPanel.darker (0.10f), centre.x, cap.getY(),
                                  kPanel.darker (0.35f),  centre.x, cap.getBottom(), false);
    g.setGradientFill (capGrad);
    g.fillEllipse (cap);

    // Pointer.
    juce::Path pointer;
    const float pl = radius * 0.82f;
    pointer.addRoundedRectangle (-dim * 0.018f, -pl, dim * 0.036f, pl * 0.46f, dim * 0.018f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (kAccent.brighter (0.35f));
    g.fillPath (pointer);
    // Glowing pointer tip.
    const juce::Point<float> tip (centre.x + std::sin (angle) * pl,
                                  centre.y - std::cos (angle) * pl);
    g.setColour (kAccent.withAlpha (0.9f));
    g.fillEllipse (juce::Rectangle<float> (dim * 0.06f, dim * 0.06f).withCentre (tip));
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

    modelBox.addItemList ({ "A - Tape", "E - EMI", "N - Neve", "T - Triode", "P - Pentode", "C - Capture" }, 1);
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

    // Preset selector, driven directly by the processor's factory presets.
    addAndMakeVisible (presetBox);
    for (int i = 0; i < proc.getNumFactoryPresets(); ++i)
        presetBox.addItem (proc.getFactoryPresetName (i), i + 1);
    presetBox.setJustificationType (juce::Justification::centred);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0 && idx != proc.getCurrentPreset())
            proc.applyPreset (idx);
    };

    addAndMakeVisible (prevPreset);
    addAndMakeVisible (nextPreset);
    prevPreset.onClick = [this]
    {
        const int n = proc.getNumFactoryPresets();
        proc.applyPreset ((proc.getCurrentPreset() - 1 + n) % n);
        refreshPresetBox();
    };
    nextPreset.onClick = [this]
    {
        const int n = proc.getNumFactoryPresets();
        proc.applyPreset ((proc.getCurrentPreset() + 1) % n);
        refreshPresetBox();
    };
    refreshPresetBox();

    // Capture loader: opens a profile.json produced by tools/extract_profile.py.
    addAndMakeVisible (loadCaptureButton);
    addAndMakeVisible (captureLabel);
    captureLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd98c3a));
    captureLabel.setFont (juce::Font (12.0f, juce::Font::italic));
    captureLabel.setJustificationType (juce::Justification::centredLeft);
    captureLabel.setText (proc.getCaptureName(), juce::dontSendNotification);
    loadCaptureButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (
            "Load a capture profile", juce::File(), "*.json");
        chooser->launchAsync (juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                const auto f = fc.getResult();
                if (f == juce::File()) return;
                if (proc.loadCaptureProfile (f))
                    captureLabel.setText ("C: " + proc.getCaptureName(), juce::dontSendNotification);
                else
                    captureLabel.setText ("load failed - not a capture profile",
                                          juce::dontSendNotification);
            });
    };

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
    presetBox.setSelectedId (proc.getCurrentPreset() + 1, juce::dontSendNotification);
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
void DecapitoneAudioProcessorEditor::drawPanel (juce::Graphics& g,
                                                juce::Rectangle<float> r,
                                                const juce::String& title)
{
    g.setColour (kPanel.withAlpha (0.55f));
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (kText.withAlpha (0.06f));
    g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

    if (title.isNotEmpty())
    {
        g.setColour (kAccent.withAlpha (0.85f));
        g.setFont (juce::Font (10.5f, juce::Font::bold));
        g.drawText (title, r.removeFromTop (16).reduced (10, 2).translated (0, 2),
                    juce::Justification::topLeft);
    }
}

void DecapitoneAudioProcessorEditor::drawMeter (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r, 3.0f);

    auto toY = [r] (float db) { return juce::jmap (juce::jlimit (-60.0f, 6.0f, db),
                                                   -60.0f, 6.0f, r.getBottom(), r.getY()); };

    // Gradient column (green -> amber -> red), clipped to the current level.
    const float db   = juce::Decibels::gainToDecibels (meterLevel, -60.0f);
    auto fill = r.withTop (toY (db));
    juce::ColourGradient grad (juce::Colour (0xff3fa34d), 0, r.getBottom(),
                               juce::Colour (0xffd23b3b), 0, r.getY(), false);
    grad.addColour (0.65, kAccent);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (fill, 3.0f);

    // dB tick marks.
    g.setColour (kText.withAlpha (0.25f));
    for (float t : { 0.0f, -6.0f, -12.0f, -24.0f, -48.0f })
    {
        const float yy = toY (t);
        g.drawLine (r.getX(), yy, r.getRight(), yy, 0.6f);
    }
    // 0 dB line emphasised.
    g.setColour (juce::Colours::red.withAlpha (0.7f));
    g.drawLine (r.getX(), toY (0.0f), r.getRight(), toY (0.0f), 1.0f);

    // Peak-hold marker.
    const float peakDb = juce::Decibels::gainToDecibels (meterPeakHold, -60.0f);
    g.setColour (peakDb > 0.0f ? juce::Colours::red : kAccent.brighter (0.3f));
    const float py = toY (peakDb);
    g.fillRect (r.getX(), py - 1.0f, r.getWidth(), 2.0f);

    // Frame.
    g.setColour (kText.withAlpha (0.12f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
}

void DecapitoneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Scale all drawing so the UI looks identical at any window size.
    const float s = (float) getWidth() / kDesignW;
    g.fillAll (kBg);
    g.addTransform (juce::AffineTransform::scale (s));

    auto full = juce::Rectangle<float> (0, 0, kDesignW, kDesignH);

    // Background: subtle vertical gradient + faint vignette.
    juce::ColourGradient bg (kBg.brighter (0.05f), 0, 0, kBg.darker (0.4f), 0, kDesignH, false);
    g.setGradientFill (bg);
    g.fillRect (full);

    // Header bar with gradient + accent underline.
    auto header = full.removeFromTop (56);
    juce::ColourGradient hg (kPanel.brighter (0.12f), 0, header.getY(),
                             kPanel.darker (0.25f), 0, header.getBottom(), false);
    g.setGradientFill (hg);
    g.fillRect (header);
    g.setColour (kAccent);
    g.fillRect (header.withTop (header.getBottom() - 2.0f));   // underline

    g.setColour (kAccent);
    g.setFont (juce::Font (29.0f, juce::Font::bold));
    g.drawText ("DECAPITONE", header.reduced (18, 0).removeFromLeft (300),
                juce::Justification::centredLeft);
    g.setColour (kText.withAlpha (0.45f));
    g.setFont (juce::Font (11.5f));
    g.drawText ("ANALOG SATURATION", header.withTrimmedRight (40).reduced (18, 0),
                juce::Justification::centredRight);

    // Backing panel behind all controls + the knob group panel.
    const float panelTop = 64.0f;
    const float panelBot = kDesignH - 16.0f;
    drawPanel (g, { 16, panelTop, kDesignW - 32 - 26, panelBot - panelTop }, {});

    // Knob group panel (lower half) with a section title.
    const float knobTop = 196.0f;
    drawPanel (g, { 24, knobTop, kDesignW - 48 - 26, panelBot - knobTop - 4 }, "CONTROLS");

    // Output meter on the right.
    drawMeter (g, juce::Rectangle<float> (kDesignW - 33, panelTop + 6, 17, panelBot - panelTop - 22));
    g.setColour (kText.withAlpha (0.45f));
    g.setFont (juce::Font (9.0f, juce::Font::bold));
    g.drawText ("OUT", juce::Rectangle<float> (kDesignW - 36, panelBot - 14, 24, 12),
                juce::Justification::centred);
}

void DecapitoneAudioProcessorEditor::resized()
{
    // Work in design coordinates, then scale the whole component tree.
    const float s = (float) getWidth() / kDesignW;

    juce::Rectangle<int> full (0, 0, (int) kDesignW, (int) kDesignH);
    full.removeFromTop (56);                 // header
    full.removeFromRight (26);               // meter gutter
    auto area = full.reduced (28, 0);
    area.removeFromTop (14);

    // Preset row.
    auto presetRow = area.removeFromTop (28);
    prevPreset.setBounds (presetRow.removeFromLeft (30));
    presetRow.removeFromLeft (5);
    presetBox.setBounds (presetRow.removeFromLeft (220));
    presetRow.removeFromLeft (5);
    nextPreset.setBounds (presetRow.removeFromLeft (30));

    area.removeFromTop (10);

    // Style + mode toggles row.
    auto top = area.removeFromTop (32);
    modelBox.setBounds (top.removeFromLeft (170).reduced (0, 2));
    top.removeFromLeft (14);
    auto toggle = [&top] (juce::ToggleButton& b) { b.setBounds (top.removeFromLeft (82).reduced (3, 2)); top.removeFromLeft (6); };
    toggle (punishButton);
    toggle (steepButton);
    toggle (thumpButton);

    area.removeFromTop (8);

    // Capture row: load button + loaded-profile name.
    auto capRow = area.removeFromTop (24);
    loadCaptureButton.setBounds (capRow.removeFromLeft (120).reduced (0, 2));
    capRow.removeFromLeft (10);
    captureLabel.setBounds (capRow);

    // Knob group sits inside the "CONTROLS" panel (top at y=196 in paint()).
    area.removeFromTop (196 - area.getY());
    area.reduce (8, 6);
    area.removeFromBottom (10);              // keep value boxes off the panel edge

    auto knobCell = [] (juce::Rectangle<int> r, LabeledKnob& k)
    {
        k.label.setBounds (r.removeFromTop (16));
        k.slider.setBounds (r.reduced (2, 0));
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
    meterLevel = target > meterLevel ? target : meterLevel * 0.82f + target * 0.18f;

    // Peak hold: jump up instantly, hold ~0.7 s, then decay.
    if (target >= meterPeakHold)
    {
        meterPeakHold = target;
        peakHoldHold  = 21;            // ~0.7 s at 30 Hz
    }
    else if (--peakHoldHold <= 0)
    {
        meterPeakHold *= 0.94f;
    }

    if (presetBox.getSelectedId() - 1 != proc.getCurrentPreset())
        refreshPresetBox();
    repaint();
}
