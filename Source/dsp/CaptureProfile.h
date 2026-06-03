#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

/*  ===========================================================================
    CaptureProfile

    A captured profile of a real saturation device (e.g. Decapitator at a
    chosen Style/Drive), produced by tools/extract_profile.py.  It holds:

      * lut      - the measured static transfer curve over [-range, +range]
      * eqMag    - the measured linear magnitude response (combined pre/post
                   emphasis + tone), on a linear frequency grid 0..Nyquist of
                   the capture sample rate
      * captureSampleRate / smallSignalGain - bookkeeping

    The plugin applies the profile as:  EQ FIR  ->  LUT waveshaper, which is the
    Wiener-Hammerstein structure these devices actually use.  Because the curve
    and EQ are *measured*, the result tracks the source device very closely
    without copying any of its code.
    =========================================================================== */

namespace decap
{

struct CaptureProfile
{
    bool                loaded = false;
    juce::String        name;
    float               range = 1.5f;
    double              captureSampleRate = 96000.0;
    float               smallSignalGain = 1.0f;
    std::vector<float>  lut;
    std::vector<float>  eqMag;   // linear magnitude, 0..Nyquist(captureSampleRate)

    // Sample the static curve with linear interpolation.
    inline float lookup (float x) const noexcept
    {
        const int n = (int) lut.size();
        if (n < 2) return x;
        float t = (x + range) / (2.0f * range) * (float) (n - 1);
        t = juce::jlimit (0.0f, (float) (n - 1), t);
        const int i = (int) t;
        const int j = juce::jmin (i + 1, n - 1);
        return lut[(size_t) i] + (lut[(size_t) j] - lut[(size_t) i]) * (t - (float) i);
    }

    // Parse a profile.json produced by the extractor.
    static bool fromJSON (const juce::var& v, CaptureProfile& out)
    {
        if (! v.isObject()) return false;
        auto* obj = v.getDynamicObject();
        if (obj == nullptr) return false;

        if (v.getProperty ("format", {}).toString() != "decapitone-capture-1")
            return false;

        out.name              = v.getProperty ("name", "captured").toString();
        out.range             = (float)  (double) v.getProperty ("lut_range", 1.5);
        out.captureSampleRate = (double) v.getProperty ("sr", 96000.0);
        out.smallSignalGain   = (float)  (double) v.getProperty ("small_signal_gain", 1.0);

        auto readArray = [] (const juce::var& arr, std::vector<float>& dst)
        {
            if (auto* a = arr.getArray())
            {
                dst.resize ((size_t) a->size());
                for (int i = 0; i < a->size(); ++i)
                    dst[(size_t) i] = (float) (double) a->getReference (i);
                return dst.size() > 1;
            }
            return false;
        };

        const bool haveLut = readArray (v.getProperty ("lut", {}), out.lut);
        readArray (v.getProperty ("eq_mag", {}), out.eqMag); // optional
        out.loaded = haveLut;
        return haveLut;
    }

    static bool fromFile (const juce::File& f, CaptureProfile& out)
    {
        if (! f.existsAsFile()) return false;
        return fromJSON (juce::JSON::parse (f.loadFileAsString()), out);
    }

    /*  Build a linear-phase FIR that matches the captured EQ magnitude at the
        host's (oversampled) sample rate.  Returns an empty vector if there is
        no EQ data, in which case the caller should skip EQ filtering.        */
    std::vector<float> buildEqFIR (double targetSampleRate, int numTaps = 257) const
    {
        std::vector<float> taps;
        if (eqMag.size() < 2) return taps;

        const int fftOrder = 11;            // 2048-point FFT
        const int fftSize  = 1 << fftOrder;
        const int nBins     = fftSize / 2 + 1;

        // Resample the stored magnitude (linear grid over the capture Nyquist)
        // onto this sample rate's bin grid.
        const double capNyq = captureSampleRate * 0.5;
        const double tgtNyq = targetSampleRate * 0.5;

        std::vector<float> mag ((size_t) nBins, 1.0f);
        for (int k = 0; k < nBins; ++k)
        {
            const double f = (double) k / (double) (nBins - 1) * tgtNyq;
            double src = juce::jlimit (0.0, 1.0, f / capNyq);   // position in eqMag
            const double pos = src * (double) (eqMag.size() - 1);
            const int i = (int) pos;
            const int j = juce::jmin (i + 1, (int) eqMag.size() - 1);
            mag[(size_t) k] = (float) (eqMag[(size_t) i] + (eqMag[(size_t) j] - eqMag[(size_t) i]) * (pos - i));
        }

        // Zero-phase spectrum -> impulse, then shift + window to linear phase.
        juce::dsp::FFT fft (fftOrder);
        std::vector<float> fd ((size_t) (fftSize * 2), 0.0f);
        for (int k = 0; k < nBins; ++k)
            fd[(size_t) (2 * k)] = mag[(size_t) k];          // real, imag = 0
        for (int k = 1; k < fftSize / 2; ++k)                 // mirror for real output
            fd[(size_t) (2 * (fftSize - k))] = mag[(size_t) k];

        fft.performRealOnlyInverseTransform (fd.data());

        numTaps = juce::jmin (numTaps | 1, fftSize - 1);      // force odd
        const int half = numTaps / 2;
        taps.resize ((size_t) numTaps);
        for (int t = -half; t <= half; ++t)
        {
            const int srcIdx = (t + fftSize) % fftSize;       // wrap (zero-phase center at 0)
            const float w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi
                              * (float) (t + half) / (float) (numTaps - 1)); // Hann
            taps[(size_t) (t + half)] = fd[(size_t) srcIdx] * w;
        }

        // Normalise DC gain to unity so the FIR only shapes tone, not level.
        double sum = 0.0; for (float v : taps) sum += v;
        if (std::abs (sum) > 1e-6) for (auto& v : taps) v /= (float) sum;
        return taps;
    }
};

} // namespace decap
