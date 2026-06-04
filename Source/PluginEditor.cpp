#include "PluginEditor.h"

namespace
{
    // Vintage brushed-metal palette, evoking classic analog saturation hardware.
    const juce::Colour kBg      { 0xff14130f }; // dark surround
    const juce::Colour kFaceTop { 0xff6f6957 }; // brushed faceplate (top)
    const juce::Colour kFaceBot { 0xff423d33 }; // brushed faceplate (bottom)
    const juce::Colour kPanel   { 0xff2b2822 }; // knob bodies / dark fields
    const juce::Colour kAccent  { 0xffc8932f }; // brass / amber value arcs
    const juce::Colour kRed     { 0xffc23b2f }; // PUNISH red
    const juce::Colour kText    { 0xfff0e8d4 }; // cream legends
    const juce::Colour kBar     { 0xff1b1915 }; // top preset bar
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

    // Pointer: vintage cream line from centre to rim.
    juce::Path pointer;
    const float pl = radius * 0.84f;
    pointer.addRoundedRectangle (-dim * 0.016f, -pl, dim * 0.032f, pl * 0.6f, dim * 0.016f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (kText);
    g.fillPath (pointer);
}

void DecapLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                         bool isHighlighted, bool /*isDown*/)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = b.getToggleState();
    // PUNISH gets the signature red; the others use brass.
    const juce::Colour onCol = b.getButtonText().containsIgnoreCase ("PUNISH") ? kRed : kAccent;

    // Recessed metal base.
    juce::ColourGradient base (kPanel.brighter (0.18f), 0, bounds.getY(),
                               kPanel.darker (0.35f), 0, bounds.getBottom(), false);
    g.setGradientFill (base);
    g.fillRoundedRectangle (bounds, 4.0f);

    if (on)
    {
        g.setColour (onCol.withAlpha (0.92f));
        g.fillRoundedRectangle (bounds.reduced (1.5f), 3.0f);
        g.setColour (onCol.brighter (0.4f).withAlpha (0.5f));   // lit rim glow
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.4f);
    }
    else
    {
        g.setColour (onCol.withAlpha (0.30f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.2f);
    }

    g.setColour (on ? juce::Colours::black.withAlpha (0.85f)
                    : kText.withAlpha (isHighlighted ? 1.0f : 0.8f));
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

    // STYLE: a 6-detent rotary (A E N T P C). Letters are drawn around it.
    modelSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    modelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (modelSlider);
    modelAttach = std::make_unique<APVTS::SliderAttachment> (proc.apvts, "model", modelSlider);

    modelLabel.setText ("STYLE", juce::dontSendNotification);
    modelLabel.setJustificationType (juce::Justification::centred);
    modelLabel.setColour (juce::Label::textColourId, kText);
    modelLabel.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (modelLabel);

    // Readout showing the selected style's full name (updates with the knob).
    styleName.setJustificationType (juce::Justification::centred);
    styleName.setColour (juce::Label::textColourId, kAccent.brighter (0.3f));
    styleName.setFont (juce::Font (13.0f, juce::Font::bold));
    addAndMakeVisible (styleName);
    modelSlider.onValueChange = [this] { updateStyleName(); };
    updateStyleName();

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

void DecapitoneAudioProcessorEditor::updateStyleName()
{
    static const char* names[] = { "TAPE", "EMI", "NEVE", "TRIODE", "PENTODE", "CAPTURE" };
    const int i = juce::jlimit (0, 5, (int) std::round (modelSlider.getValue()));
    styleName.setText (names[i], juce::dontSendNotification);
}

void DecapitoneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Scale all drawing so the UI looks identical at any window size.
    const float s = (float) getWidth() / kDesignW;
    g.fillAll (kBg);
    g.addTransform (juce::AffineTransform::scale (s));

    auto full = juce::Rectangle<float> (0, 0, kDesignW, kDesignH);

    // Top preset bar (Soundtoys-style dark strip).
    auto bar = full.removeFromTop (44);
    g.setColour (kBar);
    g.fillRect (bar);
    g.setColour (kAccent);
    g.setFont (juce::Font (22.0f, juce::Font::bold));
    g.drawText ("DECAPITONE", bar.reduced (16, 0).removeFromLeft (220),
                juce::Justification::centredLeft);
    g.setColour (kText.withAlpha (0.35f));
    g.setFont (juce::Font (10.5f));
    g.drawText ("ANALOG SATURATION", bar.removeFromRight (220).reduced (16, 0),
                juce::Justification::centredRight);

    // Brushed-metal faceplate.
    auto face = full.reduced (10.0f);
    juce::ColourGradient fg (kFaceTop, 0, face.getY(), kFaceBot, 0, face.getBottom(), false);
    g.setGradientFill (fg);
    g.fillRoundedRectangle (face, 10.0f);
    // Fine brushed lines.
    g.setColour (juce::Colours::white.withAlpha (0.02f));
    for (float yy = face.getY() + 2; yy < face.getBottom(); yy += 3.0f)
        g.drawLine (face.getX(), yy, face.getRight(), yy, 0.5f);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawRoundedRectangle (face, 10.0f, 1.2f);
    g.setColour (kAccent.withAlpha (0.25f));
    g.drawRoundedRectangle (face.reduced (2.5f), 8.0f, 1.0f);

    // Output meter on the right edge of the faceplate.
    auto meterCol = juce::Rectangle<float> (kDesignW - 34, face.getY() + 16, 16, face.getHeight() - 46);
    drawMeter (g, meterCol);
    g.setColour (kText.withAlpha (0.55f));
    g.setFont (juce::Font (9.0f, juce::Font::bold));
    g.drawText ("OUT", juce::Rectangle<float> (meterCol.getX() - 4, meterCol.getBottom() + 1, 24, 12),
                juce::Justification::centred);
}

void DecapitoneAudioProcessorEditor::resized()
{
    // Work in design coordinates, then scale the whole component tree.
    const float s = (float) getWidth() / kDesignW;

    juce::Rectangle<int> full (0, 0, (int) kDesignW, (int) kDesignH);
    full.removeFromTop (44);                 // preset bar
    auto face = full.reduced (10);           // faceplate
    face.reduce (16, 12);
    face.removeFromRight (30);               // meter gutter

    // Preset / capture row.
    auto presetRow = face.removeFromTop (26);
    prevPreset.setBounds (presetRow.removeFromLeft (28));
    presetRow.removeFromLeft (4);
    presetBox.setBounds (presetRow.removeFromLeft (190));
    presetRow.removeFromLeft (4);
    nextPreset.setBounds (presetRow.removeFromLeft (28));
    presetRow.removeFromLeft (24);
    loadCaptureButton.setBounds (presetRow.removeFromLeft (120));
    presetRow.removeFromLeft (10);
    captureLabel.setBounds (presetRow);

    // Mode toggles along the bottom; PUNISH emphasised.
    auto toggleRow = face.removeFromBottom (32);
    toggleRow.removeFromLeft (4);
    punishButton.setBounds (toggleRow.removeFromLeft (110).reduced (2));
    toggleRow.removeFromLeft (8);
    steepButton.setBounds (toggleRow.removeFromLeft (88).reduced (2));
    toggleRow.removeFromLeft (8);
    thumpButton.setBounds (toggleRow.removeFromLeft (88).reduced (2));

    face.removeFromTop (8);
    face.removeFromBottom (6);

    // Centre a compact band so each knob and its value box stay together.
    const int bandH = juce::jmin (face.getHeight(), 150);
    auto band = face.withHeight (bandH).withY (face.getCentreY() - bandH / 2);

    // One row of seven controls: DRIVE STYLE TONE LOWCUT HIGHCUT MIX OUTPUT.
    auto knobCell = [] (juce::Rectangle<int> r, juce::Label& lab, juce::Component& ctl,
                        bool /*valueBox*/)
    {
        lab.setBounds (r.removeFromTop (16));
        ctl.setBounds (r.reduced (3, 0));
    };

    const int n = 7;
    const int colW = band.getWidth() / n;
    auto col = [&] (int i) { return band.withX (band.getX() + i * colW).withWidth (colW); };

    knobCell (col (0), driveKnob.label,   driveKnob.slider,   true);
    // STYLE: caption on top, knob in the middle, full style name beneath.
    {
        auto sc = col (1);
        modelLabel.setBounds (sc.removeFromTop (16));
        styleName.setBounds (sc.removeFromBottom (18));
        modelSlider.setBounds (sc.reduced (6, 2));
    }
    knobCell (col (2), toneKnob.label,    toneKnob.slider,    true);
    knobCell (col (3), lowCutKnob.label,  lowCutKnob.slider,  true);
    knobCell (col (4), highCutKnob.label, highCutKnob.slider, true);
    knobCell (col (5), mixKnob.label,     mixKnob.slider,     true);
    knobCell (col (6), outputKnob.label,  outputKnob.slider,  true);

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
