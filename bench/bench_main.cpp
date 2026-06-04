#include "../Source/PluginProcessor.h"
#include <chrono>
#include <cstdio>

/*  Headless CPU benchmark: runs the real processBlock and reports how much CPU
    each model uses, as a fraction of real time (lower = lighter).            */
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit; // message manager for any JUCE bits

    constexpr double sr        = 48000.0;
    constexpr int    blockSize = 512;
    constexpr int    numBlocks = 4000;        // ~42.7 s of audio
    const double     audioSeconds = (double) numBlocks * blockSize / sr;

    DecapitoneAudioProcessor proc;
    proc.setPlayConfigDetails (2, 2, sr, blockSize);
    proc.prepareToPlay (sr, blockSize);

    // Optionally load a capture profile so the "C" row reflects a real,
    // loaded capture (LUT + EQ FIR) rather than empty passthrough.
    bool capLoaded = false;
    if (argc > 1)
        capLoaded = proc.loadCaptureProfile (juce::File (juce::String (argv[1])));

    juce::AudioBuffer<float> buffer (2, blockSize);
    juce::MidiBuffer midi;
    juce::Random rng;

    const char* names[] = { "A - Tape", "E - EMI", "N - Neve",
                            "T - Triode", "P - Pentode",
                            capLoaded ? "C - Capture(loaded)" : "C - Capture(empty)" };

    auto* modelParam  = proc.apvts.getParameter ("model");
    auto* driveParam  = proc.apvts.getParameter ("drive");
    auto* punishParam = proc.apvts.getParameter ("punish");

    printf("Decapitone CPU benchmark  (%.0f kHz, block %d, 4x oversampling, stereo)\n",
           sr / 1000.0, blockSize);
    printf("%-20s %10s %12s %14s\n", "Model", "CPU/real", "%1 core", "x realtime");
    printf("---------------------------------------------------------------\n");

    driveParam->setValueNotifyingHost (driveParam->convertTo0to1 (6.0f)); // hot

    double worst = 0.0;
    for (int m = 0; m < 6; ++m)
    {
        modelParam->setValueNotifyingHost ((float) m / 5.0f);

        // warmup
        for (int b = 0; b < 50; ++b)
        {
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    buffer.setSample (ch, i, rng.nextFloat() * 2.0f - 1.0f);
            proc.processBlock (buffer, midi);
        }

        const auto t0 = std::chrono::high_resolution_clock::now();
        for (int b = 0; b < numBlocks; ++b)
        {
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < blockSize; ++i)
                    buffer.setSample (ch, i, rng.nextFloat() * 2.0f - 1.0f);
            proc.processBlock (buffer, midi);
        }
        const auto t1 = std::chrono::high_resolution_clock::now();

        const double cpuSeconds = std::chrono::duration<double> (t1 - t0).count();
        const double frac = cpuSeconds / audioSeconds;     // CPU per real second
        worst = juce::jmax (worst, frac);
        printf("%-20s %9.2f%% %11.2f%% %13.1fx\n",
               names[m], frac * 100.0, frac * 100.0, 1.0 / frac);
    }

    printf("---------------------------------------------------------------\n");
    printf("Worst case: %.2f%% of one CPU core per instance.\n", worst * 100.0);
    printf("=> ~%d instances would saturate one core.\n", (int) (1.0 / worst));
    return 0;
}
