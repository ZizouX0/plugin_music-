#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// A compact dark LookAndFeel that draws chunky analog-style rotary knobs.
class DecapLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DecapLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
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
    std::unique_ptr<APVTS::ButtonAttachment> punishAttach;

    float meterLevel { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecapitoneAudioProcessorEditor)
};
