#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include <cmath>

/*  ===========================================================================
    CaptureProfile

    A captured profile of a real saturation device (e.g. Decapitator), produced
    by tools/extract_profile.py / tools/make_capture_set.py.

    Two flavours, both handled here:

      * single capture  - one static transfer curve measured at one Drive
                          setting.  The plugin's Drive knob acts as input trim.

      * capture SET     - several curves captured across a range of Drive
                          settings.  The plugin's Drive knob then moves *along*
                          the captured axis, blending between the two nearest
                          curves - so the knob behaves like the real device's.

    Each curve is a "Layer": a static transfer LUT plus the Drive value it was
    captured at.  A single linear EQ (averaged across layers) carries the
    device's tone.  Structure mirrors the real Wiener-Hammerstein topology
    (EQ around a static nonlinearity), and because everything is *measured*,
    the result tracks the source device closely without copying its code.
    =========================================================================== */

namespace decap
{

struct CaptureProfile
{
    struct Layer
    {
        float              drive = 5.0f;   // capture point on the device's Drive
        std::vector<float> lut;            // static curve over [-range, +range]

        inline float lookup (float x, float range) const noexcept
        {
            const int n = (int) lut.size();
            if (n < 2) return x;
            float t = (x + range) / (2.0f * range) * (float) (n - 1);
            t = juce::jlimit (0.0f, (float) (n - 1), t);
            const int i = (int) t;
            const int j = juce::jmin (i + 1, n - 1);
            return lut[(size_t) i] + (lut[(size_t) j] - lut[(size_t) i]) * (t - (float) i);
        }
    };

    bool                loaded = false;
    juce::String        name;
    float               range = 1.5f;
    double              captureSampleRate = 96000.0;
    std::vector<Layer>  layers;            // sorted ascending by drive
    std::vector<float>  eqMag;             // averaged linear magnitude, 0..Nyquist

    bool  isSet()       const noexcept { return layers.size() > 1; }
    float minDrive()    const noexcept { return layers.empty() ? 0.0f : layers.front().drive; }
    float maxDrive()    const noexcept { return layers.empty() ? 10.0f : layers.back().drive; }

    // Single-curve lookup (first layer). Used for non-set captures.
    inline float lookup (float x) const noexcept
    {
        return layers.empty() ? x : layers.front().lookup (x, range);
    }

    // For a set: find the two layers bracketing `drive` and the blend fraction.
    void bracket (float drive, int& i0, int& i1, float& frac) const noexcept
    {
        const int n = (int) layers.size();
        if (n <= 1) { i0 = i1 = 0; frac = 0.0f; return; }
        if (drive <= layers.front().drive) { i0 = i1 = 0; frac = 0.0f; return; }
        if (drive >= layers.back().drive)  { i0 = i1 = n - 1; frac = 0.0f; return; }
        for (int k = 0; k < n - 1; ++k)
        {
            if (drive >= layers[(size_t) k].drive && drive <= layers[(size_t) k + 1].drive)
            {
                i0 = k; i1 = k + 1;
                const float d0 = layers[(size_t) k].drive, d1 = layers[(size_t) k + 1].drive;
                frac = (d1 > d0) ? (drive - d0) / (d1 - d0) : 0.0f;
                return;
            }
        }
        i0 = i1 = n - 1; frac = 0.0f;
    }

    // Blended lookup across the captured Drive axis (cheap: two LUT taps).
    inline float lookupBlend (float x, int i0, int i1, float frac) const noexcept
    {
        const float a = layers[(size_t) i0].lookup (x, range);
        if (i0 == i1) return a;
        const float b = layers[(size_t) i1].lookup (x, range);
        return a + (b - a) * frac;
    }

    // ----- JSON loading -----------------------------------------------------
    static bool fromJSON (const juce::var& v, CaptureProfile& out)
    {
        if (! v.isObject()) return false;
        const auto fmt = v.getProperty ("format", {}).toString();

        out.name              = v.getProperty ("name", "captured").toString();
        out.range             = (float)  (double) v.getProperty ("lut_range", 1.5);
        out.captureSampleRate = (double) v.getProperty ("sr", 96000.0);
        out.layers.clear();
        out.eqMag.clear();

        auto readFloatArray = [] (const juce::var& arr, std::vector<float>& dst)
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

        if (fmt == "decapitone-captureset-1")
        {
            auto* arr = v.getProperty ("layers", {}).getArray();
            if (arr == nullptr) return false;
            std::vector<std::vector<float>> eqs;
            for (auto& lv : *arr)
            {
                Layer L;
                L.drive = (float) (double) lv.getProperty ("drive", 5.0);
                if (! readFloatArray (lv.getProperty ("lut", {}), L.lut)) continue;
                out.layers.push_back (std::move (L));
                std::vector<float> e;
                if (readFloatArray (lv.getProperty ("eq_mag", {}), e)) eqs.push_back (std::move (e));
            }
            // Average the per-layer EQ magnitudes into one tone curve.
            if (! eqs.empty())
            {
                size_t m = eqs.front().size();
                out.eqMag.assign (m, 0.0f);
                for (auto& e : eqs)
                    for (size_t i = 0; i < m && i < e.size(); ++i) out.eqMag[i] += e[i];
                for (auto& vv : out.eqMag) vv /= (float) eqs.size();
            }
        }
        else if (fmt == "decapitone-capture-1")
        {
            Layer L;
            L.drive = (float) (double) v.getProperty ("drive", 5.0);
            if (! readFloatArray (v.getProperty ("lut", {}), L.lut)) return false;
            out.layers.push_back (std::move (L));
            readFloatArray (v.getProperty ("eq_mag", {}), out.eqMag);
        }
        else
        {
            return false;
        }

        std::sort (out.layers.begin(), out.layers.end(),
                   [] (const Layer& a, const Layer& b) { return a.drive < b.drive; });
        out.loaded = ! out.layers.empty();
        return out.loaded;
    }

    static bool fromFile (const juce::File& f, CaptureProfile& out)
    {
        if (! f.existsAsFile()) return false;
        return fromJSON (juce::JSON::parse (f.loadFileAsString()), out);
    }

    /*  Linear-phase FIR matching the captured EQ magnitude at the host's
        (oversampled) rate. Empty -> caller skips EQ filtering.               */
    std::vector<float> buildEqFIR (double targetSampleRate, int numTaps = 257) const
    {
        std::vector<float> taps;
        if (eqMag.size() < 2) return taps;

        const int fftOrder = 11;
        const int fftSize  = 1 << fftOrder;
        const int nBins    = fftSize / 2 + 1;

        const double capNyq = captureSampleRate * 0.5;
        const double tgtNyq = targetSampleRate * 0.5;

        std::vector<float> mag ((size_t) nBins, 1.0f);
        for (int k = 0; k < nBins; ++k)
        {
            const double f = (double) k / (double) (nBins - 1) * tgtNyq;
            const double src = juce::jlimit (0.0, 1.0, f / capNyq);
            const double pos = src * (double) (eqMag.size() - 1);
            const int i = (int) pos;
            const int j = juce::jmin (i + 1, (int) eqMag.size() - 1);
            mag[(size_t) k] = (float) (eqMag[(size_t) i] + (eqMag[(size_t) j] - eqMag[(size_t) i]) * (pos - i));
        }

        juce::dsp::FFT fft (fftOrder);
        std::vector<float> fd ((size_t) (fftSize * 2), 0.0f);
        for (int k = 0; k < nBins; ++k)
            fd[(size_t) (2 * k)] = mag[(size_t) k];
        for (int k = 1; k < fftSize / 2; ++k)
            fd[(size_t) (2 * (fftSize - k))] = mag[(size_t) k];

        fft.performRealOnlyInverseTransform (fd.data());

        numTaps = juce::jmin (numTaps | 1, fftSize - 1);
        const int half = numTaps / 2;
        taps.resize ((size_t) numTaps);
        for (int t = -half; t <= half; ++t)
        {
            const int srcIdx = (t + fftSize) % fftSize;
            const float w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi
                              * (float) (t + half) / (float) (numTaps - 1));
            taps[(size_t) (t + half)] = fd[(size_t) srcIdx] * w;
        }

        double sum = 0.0; for (float v : taps) sum += v;
        if (std::abs (sum) > 1e-6) for (auto& v : taps) v /= (float) sum;
        return taps;
    }
};

} // namespace decap
