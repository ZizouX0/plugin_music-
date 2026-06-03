#!/usr/bin/env python3
"""
make_probe.py - generate the probe signal used to capture a saturation device.

The probe is a single mono WAV with three labelled sections, played in order:

  1. SYNC      : a short 1 kHz tone burst, used to time-align the recording.
  2. CURVE     : a slow, full-scale sine that sweeps the input amplitude across
                 the whole [-1, +1] range many times.  Pairing input vs output
                 here reveals the device's static transfer curve f(x).
  3. SWEEP     : a low-level exponential sine sweep (Farina ESS) from 20 Hz to
                 ~20 kHz.  At low level the device stays roughly linear, so this
                 measures the combined pre/post EQ (the "tone" of the model).

Workflow:
    python3 make_probe.py probe.wav
    -> load probe.wav on a track, insert Decapitator (pick a Style + Drive),
       BYPASS every other processor, render/export the wet output to a WAV,
       then run extract_profile.py probe.wav captured.wav profile.json
"""
import sys
import numpy as np
import soundfile as sf

SR          = 96000          # high rate so HF harmonics are captured cleanly
SYNC_FREQ   = 1000.0
SYNC_LEN    = 0.25
GAP         = 0.20
CURVE_FREQ  = 12.0           # slow enough to densely sample the transfer curve
CURVE_LEN   = 3.0
CURVE_AMP   = 1.0            # full scale: exercises the whole curve
SWEEP_LEN   = 4.0
SWEEP_AMP   = 0.05           # quiet: keeps the device ~linear for EQ measurement
F1, F2      = 20.0, 20000.0


def tone(freq, length, amp=0.5):
    t = np.arange(int(length * SR)) / SR
    return (amp * np.sin(2 * np.pi * freq * t)).astype(np.float32)


def silence(length):
    return np.zeros(int(length * SR), dtype=np.float32)


def ess(length, amp):
    """Farina exponential sine sweep, 20 Hz -> 20 kHz."""
    t = np.arange(int(length * SR)) / SR
    k = np.log(F2 / F1)
    phase = 2 * np.pi * F1 * length / k * (np.exp(t / length * k) - 1.0)
    return (amp * np.sin(phase)).astype(np.float32)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "probe.wav"

    parts = [
        silence(GAP),
        tone(SYNC_FREQ, SYNC_LEN, 0.5),     # SYNC marker
        silence(GAP),
        tone(CURVE_FREQ, CURVE_LEN, CURVE_AMP),  # CURVE section
        silence(GAP),
        ess(SWEEP_LEN, SWEEP_AMP),          # SWEEP section
        silence(GAP),
    ]
    sig = np.concatenate(parts)

    # Record where each section starts (seconds) so the extractor finds them.
    meta = {
        "sr": SR,
        "sync_start":  GAP,
        "sync_len":    SYNC_LEN,
        "curve_start": GAP + SYNC_LEN + GAP,
        "curve_len":   CURVE_LEN,
        "curve_freq":  CURVE_FREQ,
        "curve_amp":   CURVE_AMP,
        "sweep_start": GAP + SYNC_LEN + GAP + CURVE_LEN + GAP,
        "sweep_len":   SWEEP_LEN,
        "f1": F1, "f2": F2,
    }
    sf.write(out, sig, SR, subtype="FLOAT")

    import json
    with open(out + ".meta.json", "w") as f:
        json.dump(meta, f, indent=2)

    print(f"Wrote {out}  ({len(sig)/SR:.2f} s @ {SR} Hz)")
    print(f"Wrote {out}.meta.json")
    print("\nNext:")
    print("  1. Play this WAV through Decapitator (set Style + Drive, no other FX).")
    print("  2. Export the WET output to captured.wav (same length, same SR).")
    print("  3. python3 extract_profile.py probe.wav captured.wav profile.json")


if __name__ == "__main__":
    main()
