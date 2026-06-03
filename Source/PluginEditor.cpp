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
    setColour (juce::PopupMenu::backgroundColourId, kPanel);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kAccent);
    setColour (juce::ToggleButton::textColourId, kText);
}

void DecapLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width,
                                         int height, float sliderPos,
                                         float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + sliderPos * (endAngle - startAngle);

    // Body
    g.setColour (kPanel.brighter (0.08f));
    g.fillEllipse (bounds);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (bounds, 1.5f);

    // Value arc
    juce::Path arc;
    const float arcR = radius - 2.0f;
    arc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (kAccent);
    g.strokePath (arc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // Pointer
    juce::Path pointer;
    const float pl = radius * 0.7f;
    pointer.addRoundedRectangle (-1.5f, -pl, 3.0f, pl * 0.55f, 1.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (kText);
    g.fillPath (pointer);
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

    punishButton.setClickingTogglesState (true);
    addAndMakeVisible (punishButton);
    punishAttach = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, "punish", punishButton);

    setSize (560, 320);
    startTimerHz (30);
}

DecapitoneAudioProcessorEditor::~DecapitoneAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void DecapitoneAudioProcessorEditor::setUpKnob (LabeledKnob& k, const juce::String& id,
                                                const juce::String& text)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
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
    g.fillAll (kBg);

    // Header bar
    auto header = getLocalBounds().removeFromTop (52).toFloat();
    g.setColour (kPanel);
    g.fillRect (header);
    g.setColour (kAccent);
    g.setFont (juce::Font (26.0f, juce::Font::bold));
    g.drawText ("DECAPITONE", header.reduced (16, 0).removeFromLeft (260),
                juce::Justification::centredLeft);
    g.setColour (kText.withAlpha (0.6f));
    g.setFont (juce::Font (12.0f));
    g.drawText ("analog saturation", header.reduced (16, 0),
                juce::Justification::centredRight);

    // Output meter (right edge)
    auto meterArea = getLocalBounds().toFloat().removeFromRight (18).reduced (4, 60);
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
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (52);          // header
    area.removeFromRight (24);        // meter gutter

    // Top row: style selector + punish.
    auto top = area.removeFromTop (40);
    modelBox.setBounds (top.removeFromLeft (220).reduced (0, 6));
    top.removeFromLeft (16);
    punishButton.setBounds (top.removeFromLeft (120).reduced (0, 4));

    area.removeFromTop (12);

    // Two rows of three knobs.
    auto knobRow = [] (juce::Rectangle<int> r, LabeledKnob& k)
    {
        k.label.setBounds (r.removeFromTop (16));
        k.slider.setBounds (r);
    };

    const int knobW = area.getWidth() / 3;
    auto row1 = area.removeFromTop (area.getHeight() / 2);
    auto row2 = area;

    knobRow (row1.removeFromLeft (knobW).reduced (6), driveKnob);
    knobRow (row1.removeFromLeft (knobW).reduced (6), toneKnob);
    knobRow (row1.removeFromLeft (knobW).reduced (6), mixKnob);

    knobRow (row2.removeFromLeft (knobW).reduced (6), lowCutKnob);
    knobRow (row2.removeFromLeft (knobW).reduced (6), highCutKnob);
    knobRow (row2.removeFromLeft (knobW).reduced (6), outputKnob);
}

void DecapitoneAudioProcessorEditor::timerCallback()
{
    const float target = proc.outputLevel.load();
    // Simple ballistic smoothing for a nicer meter.
    meterLevel = target > meterLevel ? target : meterLevel * 0.85f + target * 0.15f;
    repaint();
}
