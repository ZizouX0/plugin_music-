#pragma once

#include <JuceHeader.h>
#include "dsp/Saturation.h"

/*  ===========================================================================
    Decapitone - an analog saturation / distortion processor.

    Signal flow (per channel, inside 4x oversampling around the nonlinearity):

        in -> [low cut HPF] -> [tone tilt] -> drive gain ->
              [ STYLE WAVESHAPER ] -> [high cut LPF] ->
              auto-gain comp -> output gain -> [dry/wet mix] -> out

    "Punish" multiplies the effective drive for extreme, broken textures.
    =========================================================================== */

class DecapitoneAudioProcessor : public juce::AudioProcessor
{
public:
    DecapitoneAudioProcessor();
    ~DecapitoneAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public so the editor can attach to it.
    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Lightweight output level for a GUI meter (atomic, read by the editor).
    std::atomic<float> outputLevel { 0.0f };

private:
    static constexpr int oversampleFactor = 2; // 2 -> 4x oversampling

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // Per-channel filter state.
    using Filter = juce::dsp::IIR::Filter<float>;
    std::array<Filter, 2> lowCut;     // high-pass
    std::array<Filter, 2> highCut;    // low-pass
    std::array<Filter, 2> toneLow;    // tilt: low shelf
    std::array<Filter, 2> toneHigh;   // tilt: high shelf

    double currentSampleRate { 44100.0 };

    // Smoothed values to avoid zipper noise on automation.
    juce::SmoothedValue<float> driveSmoothed, mixSmoothed, outputSmoothed;

    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecapitoneAudioProcessor)
};
