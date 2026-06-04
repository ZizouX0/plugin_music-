#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// A dark, "skinned" LookAndFeel that draws polished analog-style rotary knobs
// (drop shadow, brushed-metal gradient, tick ring, amber value arc) plus tidy
// toggle buttons - all vector-drawn so there are no external image assets.
class DecapLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DecapLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;
};

//==============================================================================
class DecapitoneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit DecapitoneAudioProcessorEditor (DecapitoneAudioProcessor&);
    ~DecapitoneAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshPresetBox();

    using APVTS = juce::AudioProcessorValueTreeState;

    struct LabeledKnob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<APVTS::SliderAttachment> attach;
    };

    void setUpKnob (LabeledKnob&, const juce::String& paramID, const juce::String& text);

    DecapitoneAudioProcessor& proc;
    DecapLookAndFeel lnf;

    LabeledKnob driveKnob, toneKnob, lowCutKnob, highCutKnob, mixKnob, outputKnob;

    // Signature STYLE control: a rotary that snaps through the six styles, with
    // the selected style's full name shown beneath it.
    juce::Slider modelSlider;
    juce::Label  modelLabel;   // "STYLE" caption
    juce::Label  styleName;    // current style name readout
    std::unique_ptr<APVTS::SliderAttachment> modelAttach;
    void updateStyleName();

    juce::ToggleButton punishButton { "PUNISH" };
    juce::ToggleButton steepButton  { "STEEP" };
    juce::ToggleButton thumpButton  { "THUMP" };
    std::unique_ptr<APVTS::ButtonAttachment> punishAttach, steepAttach, thumpAttach;

    // Preset selector (driven via host programs).
    juce::ComboBox   presetBox;
    juce::TextButton prevPreset { "<" }, nextPreset { ">" };

    // Capture engine UI.
    juce::TextButton loadCaptureButton { "LOAD CAPTURE" };
    juce::Label      captureLabel;
    std::unique_ptr<juce::FileChooser> chooser;

    float meterLevel    { 0.0f };
    float meterPeakHold { 0.0f };
    int   peakHoldHold  { 0 };

    void drawMeter (juce::Graphics&, juce::Rectangle<float>);
    void drawPanel (juce::Graphics&, juce::Rectangle<float>, const juce::String& title);

    // Reference design size; the window scales from this while keeping ratio.
    static constexpr float kDesignW = 760.0f;
    static constexpr float kDesignH = 380.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecapitoneAudioProcessorEditor)
};
