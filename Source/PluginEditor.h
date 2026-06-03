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

    juce::ComboBox modelBox;
    std::unique_ptr<APVTS::ComboBoxAttachment> modelAttach;

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

    float meterLevel { 0.0f };

    // Reference design size; the window scales from this while keeping ratio.
    static constexpr float kDesignW = 600.0f;
    static constexpr float kDesignH = 470.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecapitoneAudioProcessorEditor)
};
