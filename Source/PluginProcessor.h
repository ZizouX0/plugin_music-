#pragma once

#include <JuceHeader.h>
#include "dsp/Saturation.h"
#include "dsp/CaptureProfile.h"

/*  ===========================================================================
    Decapitone - an analog saturation / distortion processor.

    Signal flow (per channel, inside 4x oversampling around the nonlinearity):

        in -> [low cut HPF (+steep 4th-order)] -> [tone tilt] ->
              [per-model PRE-emphasis] -> drive gain ->
              [ STYLE WAVESHAPER ] * makeup -> [per-model DE-emphasis] ->
              [high cut LPF] -> [Thump low shelf] ->
              mix-aware auto-gain -> output gain -> [dry/wet mix] -> out

    "Punish" multiplies the effective drive for extreme, broken textures.
    "Steep" doubles the low-cut slope; "Thump" adds low-end weight on output.
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

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public so the editor can attach to it.
    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Lightweight output level for a GUI meter (atomic, read by the editor).
    std::atomic<float> outputLevel { 0.0f };

    // --- Capture engine (model "C") -----------------------------------------
    // Load a profile.json captured from a real device. Real-time safe: the new
    // profile is parsed on the message thread into an inactive slot, then the
    // active index is flipped atomically for the audio thread to pick up.
    bool loadCaptureProfile (const juce::File& file);
    juce::String getCaptureName() const;          // for the GUI
    juce::File   getCaptureFile() const { return captureFile; }

private:
    static constexpr int oversampleFactor = 2; // 2 -> 4x oversampling

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // Per-channel filter state.
    using Filter = juce::dsp::IIR::Filter<float>;
    std::array<Filter, 2> lowCut;     // high-pass (1st stage)
    std::array<Filter, 2> lowCut2;    // high-pass (2nd stage, engaged by "Steep")
    std::array<Filter, 2> highCut;    // low-pass
    std::array<Filter, 2> toneLow;    // tilt: low shelf
    std::array<Filter, 2> toneHigh;   // tilt: high shelf
    std::array<Filter, 2> preEmph;    // per-model pre-emphasis (peaking)
    std::array<Filter, 2> postEmph;   // per-model de-emphasis (peaking, inverse)
    std::array<Filter, 2> thump;      // low-shelf weight on output

    // Capture engine state.
    std::array<std::shared_ptr<decap::CaptureProfile>, 2> captureSlots;
    std::atomic<int> activeCaptureSlot { 0 };
    std::array<juce::dsp::FIR::Filter<float>, 2> captureEq; // per-channel EQ FIR
    juce::dsp::FIR::Coefficients<float>::Ptr captureEqCoeffs;
    juce::File captureFile;
    void rebuildCaptureEq (const decap::CaptureProfile& p);

    double currentSampleRate { 44100.0 };
    int    currentProgram    { 0 };

    // Smoothed values to avoid zipper noise on automation.
    juce::SmoothedValue<float> driveSmoothed, mixSmoothed, outputSmoothed;

    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecapitoneAudioProcessor)
};
