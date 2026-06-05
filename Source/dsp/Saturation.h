#pragma once

#include <cmath>

/*  ===========================================================================
    Saturation.h

    The analog colour engine.  Five voiced waveshapers loosely modelled on the
    circuits that inspired the classic Soundtoys Decapitator:

        A  - Ampex 350 tape preamp   : soft, warm, gentle even-harmonic glow
        E  - EMI TG12345 console     : punchy, brighter, controlled odd grit
        N  - Neve 1057 input         : creamy, thick even harmonics
        T  - Triode tube             : asymmetric, fat 2nd harmonic
        P  - Pentode tube            : aggressive, buzzy odd harmonics

    Each model is a static transfer function `shape(x)` operating on a single
    sample that has already been driven (gained) by the caller.  Keeping them
    branch-light and stateless makes them trivial to run inside an oversampled
    loop.

    Alongside the transfer function, every model carries a `voicing()` profile:
    a pre-emphasis filter pushed into the waveshaper and a complementary
    de-emphasis afterwards (so the steady-state response stays roughly flat but
    the harmonics are generated in a model-specific band), plus a makeup factor
    that normalises the loudness of the five models against each other.
    =========================================================================== */

namespace decap
{

enum class Model
{
    tapeA = 0,   // Ampex 350
    emiE,        // EMI TG12345
    neveN,       // Neve 1057
    triodeT,     // Triode tube
    pentodeP,    // Pentode tube
    captureC,    // loaded capture profile (LUT waveshaper)
    numModels
};

// Per-model tone shaping applied around the nonlinearity.
struct Voicing
{
    float emphasisHz;   // centre of the pre/de-emphasis band
    float emphasisQ;    // bandwidth of that band
    float emphasisDb;   // pre boost (post applies the negative of this)
    float makeupDb;     // loudness match so all five models sit at similar level
};

inline Voicing voicing (Model model) noexcept
{
    switch (model)
    {
        // Emphasis is deliberately gentle: boosting a band hard into the
        // waveshaper makes vocals fizzy/harsh, so these are light touches.
        // Tape: a slight upper-mid lift below the harsh sibilance region.
        case Model::tapeA:    return { 2200.0f, 0.45f, 1.5f,  0.0f };
        // EMI: mild presence emphasis to stay articulate.
        case Model::emiE:     return { 2000.0f, 0.50f, 2.0f, -1.5f };
        // Neve: low-mid emphasis -> thick, chesty console weight.
        case Model::neveN:    return {  300.0f, 0.55f, 2.5f,  4.0f };
        // Triode: gentle mid emphasis for a forward tube sound.
        case Model::triodeT:  return { 1000.0f, 0.45f, 1.5f, -0.5f };
        // Pentode: a touch of upper-mid bite (kept modest to avoid fizz).
        case Model::pentodeP: return { 2600.0f, 0.60f, 2.5f,  3.0f };
        default:              return { 1000.0f, 0.5f,  0.0f,  0.0f };
    }
}

// A fast, smooth saturating core used as a building block by several models.
inline float softClip (float x) noexcept
{
    // Cubic soft clip with a hard ceiling; cheaper than tanh, very musical.
    if (x <= -1.0f) return -2.0f / 3.0f;
    if (x >=  1.0f) return  2.0f / 3.0f;
    return x - (x * x * x) / 3.0f;
}

inline float tubeAsym (float x, float bias) noexcept
{
    // Add a small DC-style bias before a tanh so the two halves of the wave
    // saturate differently -> strong even-harmonic ("tube warmth") content.
    const float y = std::tanh (x + bias) - std::tanh (bias);
    return y;
}

/*  shape()
    @param x      input sample (already multiplied by the drive amount)
    @param model  which analog voicing to use
    @returns      saturated sample, roughly unity-scaled so downstream
                  auto-gain can compensate predictably.
*/
inline float shape (float x, Model model) noexcept
{
    switch (model)
    {
        case Model::tapeA:
        {
            // Ampex tape: gentle tanh with a hint of asymmetry. Rounds peaks
            // without ever sounding harsh; the classic "glue" voicing.
            return tubeAsym (x * 0.9f, 0.12f) * 1.08f;
        }

        case Model::emiE:
        {
            // EMI console: arctan gives a slightly harder knee than tanh, so
            // it stays articulate and bright as it pushes into clip.
            const float k = 1.3f;
            return (std::atan (x * k) / std::atan (k)) * 0.96f;
        }

        case Model::neveN:
        {
            // Neve: heavily even-harmonic. A bigger bias and a softer curve
            // give the thick, creamy low-mid push these consoles are loved for.
            const float y = tubeAsym (x * 0.8f, 0.28f);
            return softClip (y * 1.15f) * 1.1f;
        }

        case Model::triodeT:
        {
            // Triode: asymmetric tube. Positive and negative halves use a
            // different exponent, producing a fat dominant 2nd harmonic.
            if (x >= 0.0f)
                return (1.0f - std::exp (-x)) * 0.9f;
            else
                return -(1.0f - std::exp (x * 0.6f)) / 0.6f * 0.9f;
        }

        case Model::pentodeP:
        {
            // Pentode: harder, odd-harmonic biased, with a cubic "edge" added
            // on top of a tanh core for that gnarly, buzzy character.
            const float core = std::tanh (x * 1.4f);
            const float edge = 0.15f * core * core * core;
            return (core - edge) * 0.95f;
        }

        default:
            return std::tanh (x);
    }
}

} // namespace decap
