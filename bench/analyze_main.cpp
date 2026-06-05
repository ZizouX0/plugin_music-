#include "../Source/PluginProcessor.h"
#include <cstdio>
#include <vector>
#include <cmath>

/*  Objective audio-quality probe: runs a pure sine through the real processor
    and reports DC offset (the "woofy/pumping" culprit) and inharmonic/aliasing
    energy (the "fizzy/harsh" culprit) for each model at the default drive.   */
int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const double sr = 48000.0;
    const int    block = 512;
    DecapitoneAudioProcessor proc;
    proc.setPlayConfigDetails (2, 2, sr, block);
    proc.prepareToPlay (sr, block);

    auto* modelP = proc.apvts.getParameter ("model");
    auto* driveP = proc.apvts.getParameter ("drive");
    driveP->setValueNotifyingHost (driveP->convertTo0to1 (3.0f));   // default drive

    const char* names[] = { "Tape", "EMI", "Neve", "Triode", "Pentode" };
    const double f0 = 1000.0;
    const int N = 1 << 15;                       // analysis length
    const int fftOrder = 15;
    juce::dsp::FFT fft (fftOrder);

    printf("Per-model probe: 1 kHz sine @ -10 dBFS, Drive 3 (default)\n");
    printf("%-9s %14s %16s\n", "Model", "DC offset", "inharmonic/alias");
    printf("---------------------------------------------------\n");

    for (int m = 0; m < 5; ++m)
    {
        modelP->setValueNotifyingHost ((float) m / 5.0f);

        juce::AudioBuffer<float> buf (2, block);
        juce::MidiBuffer midi;
        std::vector<float> out; out.reserve (N + 8 * block);

        // run long enough to flush latency + fill N samples
        double phase = 0.0;
        const double amp = 0.3162;              // -10 dBFS
        int produced = 0;
        const int totalBlocks = (N + 8 * block) / block + 1;
        for (int b = 0; b < totalBlocks; ++b)
        {
            for (int i = 0; i < block; ++i)
            {
                const float s = (float) (amp * std::sin (phase));
                phase += 2.0 * juce::MathConstants<double>::pi * f0 / sr;
                buf.setSample (0, i, s); buf.setSample (1, i, s);
            }
            proc.processBlock (buf, midi);
            for (int i = 0; i < block; ++i) out.push_back (buf.getSample (0, i));
            produced += block;
        }

        // take the tail N samples (steady state, past latency)
        std::vector<float> fd ((size_t) (2 * (1 << fftOrder)), 0.0f);
        const int start = (int) out.size() - N;
        double mean = 0.0;
        for (int i = 0; i < N; ++i) mean += out[(size_t)(start + i)];
        mean /= N;                              // DC offset
        std::fill (fd.begin(), fd.end(), 0.0f);
        for (int i = 0; i < N; ++i)
        {
            const float w = 0.5f - 0.5f * std::cos (2.0f * juce::MathConstants<float>::pi * i / (N - 1));
            fd[(size_t) i] = (out[(size_t)(start + i)]) * w;   // real input in [0..N-1]
        }
        fft.performFrequencyOnlyForwardTransform (fd.data());  // magnitudes in [0..N/2]

        const double binHz = sr / N;
        const int fundBin = (int) std::round (f0 / binHz);
        double fundE = 0, harmE = 0, otherE = 0;
        for (int k = 1; k < N / 2; ++k)
        {
            const double e = (double) fd[(size_t) k] * fd[(size_t) k];
            const double hz = k * binHz;
            const double r = hz / f0;
            const bool nearInt = std::abs (r - std::round (r)) < (3.0 * binHz / f0);
            if (k >= fundBin - 2 && k <= fundBin + 2) fundE += e;
            else if (nearInt && std::round (r) >= 2)  harmE += e;
            else                                       otherE += e;
        }
        const double inharmDb = 10.0 * std::log10 ((otherE + 1e-20) / (fundE + 1e-20));
        printf("%-9s %12.5f   %12.1f dB\n", names[m], mean, inharmDb);
    }
    printf("\n(DC offset should be ~0.000; inharmonic/alias more negative = cleaner)\n");
    return 0;
}
