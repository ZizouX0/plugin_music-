#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/Presets.h"

namespace
{
    // Parameter IDs kept in one place so the editor and processor agree.
    constexpr auto pDrive   = "drive";
    constexpr auto pModel   = "model";
    constexpr auto pTone    = "tone";
    constexpr auto pLowCut  = "lowcut";
    constexpr auto pHighCut = "highcut";
    constexpr auto pPunish  = "punish";
    constexpr auto pSteep   = "steep";
    constexpr auto pThump   = "thump";
    constexpr auto pMix     = "mix";
    constexpr auto pOutput  = "output";
}

//==============================================================================
DecapitoneAudioProcessor::DecapitoneAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DecapitoneAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    // Drive: 0..10 like the hardware-style "amount" knob.
    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pDrive, 1 }, "Drive",
        NormalisableRange<float> (0.0f, 10.0f, 0.01f), 3.0f));

    layout.add (std::make_unique<AudioParameterChoice>(
        ParameterID { pModel, 1 }, "Style",
        StringArray { "A - Tape", "E - EMI", "N - Neve", "T - Triode", "P - Pentode", "C - Capture" },
        0));

    // Tone: -1 dark .. +1 bright tilt EQ, 0 = neutral.
    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pTone, 1 }, "Tone",
        NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));

    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pLowCut, 1 }, "Low Cut",
        NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.3f), 20.0f,
        AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pHighCut, 1 }, "High Cut",
        NormalisableRange<float> (1000.0f, 20000.0f, 1.0f, 0.3f), 20000.0f,
        AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<AudioParameterBool>(
        ParameterID { pPunish, 1 }, "Punish", false));

    layout.add (std::make_unique<AudioParameterBool>(
        ParameterID { pSteep, 1 }, "Steep", false));

    layout.add (std::make_unique<AudioParameterBool>(
        ParameterID { pThump, 1 }, "Thump", false));

    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pMix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<AudioParameterFloat>(
        ParameterID { pOutput, 1 }, "Output",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

//==============================================================================
void DecapitoneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2, oversampleFactor,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    oversampler->initProcessing (static_cast<size_t> (samplesPerBlock));

    // Report the oversampler's latency so the host can compensate (PDC).
    setLatencySamples (juce::roundToInt (oversampler->getLatencyInSamples()));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate * oversampler->getOversamplingFactor();
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock) * oversampler->getOversamplingFactor();
    spec.numChannels      = 1;

    for (auto* bank : { &lowCut, &lowCut2, &highCut, &toneLow, &toneHigh,
                        &preEmph, &postEmph, &thump })
        for (auto& f : *bank)
            f.prepare (spec);

    juce::dsp::ProcessSpec monoSpec { spec.sampleRate, spec.maximumBlockSize, 1 };
    for (auto& f : captureEq)
        f.prepare (monoSpec);

    // If a capture is already loaded (e.g. restored from state), design its EQ
    // for this (oversampled) rate.
    if (auto p = captureSlots[(size_t) activeCaptureSlot.load()])
        if (p->loaded)
            rebuildCaptureEq (*p);

    driveSmoothed .reset (sampleRate, 0.02);
    mixSmoothed   .reset (sampleRate, 0.02);
    outputSmoothed.reset (sampleRate, 0.02);

    updateFilters();
}

void DecapitoneAudioProcessor::updateFilters()
{
    const double osRate = currentSampleRate * (oversampler ? oversampler->getOversamplingFactor() : 1);

    const float lc    = apvts.getRawParameterValue (pLowCut)->load();
    const float hc    = apvts.getRawParameterValue (pHighCut)->load();
    const float tone  = apvts.getRawParameterValue (pTone)->load();
    const auto  model = static_cast<decap::Model> ((int) apvts.getRawParameterValue (pModel)->load());
    const auto  voice = decap::voicing (model);

    auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass (osRate, lc);
    auto lp = juce::dsp::IIR::Coefficients<float>::makeLowPass  (osRate, hc);

    // Tone is a tilt: positive boosts highs / cuts lows, negative the reverse.
    const float tiltDb = tone * 6.0f; // +/- 6 dB at the extremes
    auto ls = juce::dsp::IIR::Coefficients<float>::makeLowShelf  (osRate, 250.0f,  0.5f, juce::Decibels::decibelsToGain (-tiltDb));
    auto hs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (osRate, 4000.0f, 0.5f, juce::Decibels::decibelsToGain ( tiltDb));

    // Per-model emphasis: pre boosts the band into the waveshaper, post undoes
    // it so the net linear response is flat but the harmonics are voiced.
    auto preC  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                    osRate, voice.emphasisHz, voice.emphasisQ,
                    juce::Decibels::decibelsToGain ( voice.emphasisDb));
    auto postC = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                    osRate, voice.emphasisHz, voice.emphasisQ,
                    juce::Decibels::decibelsToGain (-voice.emphasisDb));

    // Thump: fixed low-shelf weight added on output when engaged.
    auto thC = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                    osRate, 110.0f, 0.7f, juce::Decibels::decibelsToGain (6.0f));

    for (auto& f : lowCut)   *f.coefficients = *hp;
    for (auto& f : lowCut2)  *f.coefficients = *hp;
    for (auto& f : highCut)  *f.coefficients = *lp;
    for (auto& f : toneLow)  *f.coefficients = *ls;
    for (auto& f : toneHigh) *f.coefficients = *hs;
    for (auto& f : preEmph)  *f.coefficients = *preC;
    for (auto& f : postEmph) *f.coefficients = *postC;
    for (auto& f : thump)    *f.coefficients = *thC;
}

//==============================================================================
bool DecapitoneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void DecapitoneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = buffer.getNumChannels();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    updateFilters();

    const auto model    = static_cast<decap::Model> ((int) apvts.getRawParameterValue (pModel)->load());
    const bool punish   = apvts.getRawParameterValue (pPunish)->load() > 0.5f;
    const bool steep    = apvts.getRawParameterValue (pSteep)->load()  > 0.5f;
    const bool useThump = apvts.getRawParameterValue (pThump)->load()  > 0.5f;
    const float driveRaw = apvts.getRawParameterValue (pDrive)->load();
    const float mixRaw   = apvts.getRawParameterValue (pMix)->load() * 0.01f;
    const float outRaw   = juce::Decibels::decibelsToGain (apvts.getRawParameterValue (pOutput)->load());

    // Map 0..10 to a usable gain. Punish opens up a far hotter range.
    const float driveGain = juce::Decibels::decibelsToGain (driveRaw * (punish ? 6.0f : 3.0f));

    // Mix-aware auto-gain: tame the wet path so its loudness stays close to the
    // dry signal regardless of drive, then apply the per-model loudness match.
    // Because the wet is normalised *before* the dry/wet blend, the perceived
    // level holds steady as you sweep Mix.
    const float makeup   = juce::Decibels::decibelsToGain (decap::voicing (model).makeupDb);
    const float autoComp = makeup / std::sqrt (1.0f + driveGain * 0.25f);

    driveSmoothed .setTargetValue (driveGain);
    mixSmoothed   .setTargetValue (mixRaw);
    outputSmoothed.setTargetValue (outRaw);

    // Keep a dry copy for the mix.
    juce::AudioBuffer<float> dry;
    dry.makeCopyOf (buffer);

    // Capture engine: pick up the active profile once per block (RT-safe).
    const bool useCapture = (model == decap::Model::captureC);
    decap::CaptureProfile* cap = useCapture ? captureSlots[(size_t) activeCaptureSlot.load()].get() : nullptr;
    const bool capReady = useCapture && cap != nullptr && cap->loaded;
    const bool capHasEq = capReady && captureEqCoeffs != nullptr;

    // Single capture: Drive is input trim around the captured point.
    // Capture SET: Drive moves along the captured axis, blending two curves.
    const bool capIsSet = capReady && cap->isSet();
    int   capI0 = 0, capI1 = 0;
    float capFrac = 0.0f;
    if (capIsSet)
        cap->bracket (juce::jmap (driveRaw, 0.0f, 10.0f, cap->minDrive(), cap->maxDrive()),
                      capI0, capI1, capFrac);
    const float captureInGain = capIsSet ? 1.0f : (driveRaw / 3.0f);

    juce::dsp::AudioBlock<float> block (buffer);
    auto osBlock = oversampler->processSamplesUp (block);

    const int osNumSamples = (int) osBlock.getNumSamples();

    for (int ch = 0; ch < numCh && ch < 2; ++ch)
    {
        auto* data = osBlock.getChannelPointer ((size_t) ch);

        for (int i = 0; i < osNumSamples; ++i)
        {
            float x = data[i];

            // Low cut (Steep engages a second stage for a 4th-order slope).
            x = lowCut[ch].processSample (x);
            if (steep)
                x = lowCut2[ch].processSample (x);

            // Global tilt tone.
            x = toneLow[ch] .processSample (x);
            x = toneHigh[ch].processSample (x);

            if (useCapture)
            {
                // Measured static curve (blended across Drive for a set),
                // then the measured tone EQ.
                if (capReady)
                {
                    x = capIsSet ? cap->lookupBlend (x * captureInGain, capI0, capI1, capFrac)
                                 : cap->lookup (x * captureInGain);
                    if (capHasEq)
                        x = captureEq[ch].processSample (x);
                }
                // (no profile loaded -> pass through cleanly)
            }
            else
            {
                // Per-model pre-emphasis -> drive -> waveshaper -> de-emphasis.
                x = preEmph[ch].processSample (x);
                const float d = driveSmoothed.getNextValue();
                x = decap::shape (x * d, model) * autoComp;
                x = postEmph[ch].processSample (x);
            }

            // High cut, then optional low-end Thump.
            x = highCut[ch].processSample (x);
            if (useThump)
                x = thump[ch].processSample (x);

            data[i] = x;
        }
    }

    oversampler->processSamplesDown (block);

    // Mix + output gain at base rate.
    float peak = 0.0f;
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* wet = buffer.getWritePointer (ch);
        const auto* dryData = dry.getReadPointer (juce::jmin (ch, dry.getNumChannels() - 1));

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float m = mixSmoothed.getNextValue();
            const float g = outputSmoothed.getNextValue();
            float s = (wet[i] * m + dryData[i] * (1.0f - m)) * g;
            wet[i] = s;
            peak = juce::jmax (peak, std::abs (s));
        }
    }

    outputLevel.store (peak);
}

//==============================================================================
// Factory presets - recalled cleanly: every parameter is reset to its default
// first, then the preset's named values are applied. Driven by the editor, not
// the host program interface (exposing presets as programs lets a host reset
// parameters underneath state restoration).
int DecapitoneAudioProcessor::getNumFactoryPresets() const
{
    return (int) decap::factoryPresets().size();
}

void DecapitoneAudioProcessor::applyPreset (int index)
{
    const auto& presets = decap::factoryPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    currentPreset = index;

    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : presets[(size_t) index].values)
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
}

juce::String DecapitoneAudioProcessor::getFactoryPresetName (int index) const
{
    const auto& presets = decap::factoryPresets();
    if (juce::isPositiveAndBelow (index, (int) presets.size()))
        return presets[(size_t) index].name;
    return {};
}

juce::String DecapitoneAudioProcessor::getFactoryPresetCategory (int index) const
{
    const auto& presets = decap::factoryPresets();
    if (juce::isPositiveAndBelow (index, (int) presets.size()) && presets[(size_t) index].category != nullptr)
        return presets[(size_t) index].category;
    return {};
}

//==============================================================================
// Capture engine.
bool DecapitoneAudioProcessor::loadCaptureProfile (const juce::File& file)
{
    auto profile = std::make_shared<decap::CaptureProfile>();
    if (! decap::CaptureProfile::fromFile (file, *profile))
        return false;

    rebuildCaptureEq (*profile);

    // Publish into the inactive slot, then flip the active index atomically.
    const int inactive = activeCaptureSlot.load() ^ 1;
    captureSlots[(size_t) inactive] = profile;
    activeCaptureSlot.store (inactive);
    captureFile = file;
    return true;
}

void DecapitoneAudioProcessor::rebuildCaptureEq (const decap::CaptureProfile& p)
{
    const double osRate = currentSampleRate * (oversampler ? oversampler->getOversamplingFactor() : 1);
    auto taps = p.buildEqFIR (osRate, 257);
    if (taps.empty())
    {
        captureEqCoeffs = nullptr;
        return;
    }

    captureEqCoeffs = new juce::dsp::FIR::Coefficients<float> (taps.data(), taps.size());
    for (auto& f : captureEq)
    {
        f.coefficients = captureEqCoeffs;
        f.reset();
    }
}

juce::String DecapitoneAudioProcessor::getCaptureName() const
{
    if (auto p = captureSlots[(size_t) activeCaptureSlot.load()])
        if (p->loaded)
            return p->name;
    return "(no capture loaded)";
}

//==============================================================================
juce::AudioProcessorEditor* DecapitoneAudioProcessor::createEditor()
{
    return new DecapitoneAudioProcessorEditor (*this);
}

void DecapitoneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        // Remember which capture profile was loaded so it returns on reload.
        state.setProperty ("captureFile", captureFile.getFullPathName(), nullptr);
        juce::MemoryOutputStream mos (destData, true);
        state.writeToStream (mos);
    }
}

void DecapitoneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    if (tree.isValid())
    {
        const auto path = tree.getProperty ("captureFile", "").toString();
        apvts.replaceState (tree);
        if (path.isNotEmpty())
            loadCaptureProfile (juce::File (path));
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DecapitoneAudioProcessor();
}
