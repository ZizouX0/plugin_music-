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

    Harmonic content is sculpted three ways:
      * curve hardness  -> how quickly the function saturates
      * asymmetry       -> bias between the positive and negative halves, which
                           is what actually generates *even* harmonics
      * a touch of cubic shaping for the tube models, which is where the
        musical "thickness" lives.
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
    numModels
};

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
