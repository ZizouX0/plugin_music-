#!/usr/bin/env python3
"""
selftest.py - validate the capture pipeline against a KNOWN device.

We don't need Decapitator to prove the maths works: we build a synthetic device
with a transfer curve we choose ourselves, push the probe through it, run the
extractor, and check the reconstructed curve matches the ground truth.

If the reconstruction error is tiny, the same pipeline will faithfully capture
a real device (Decapitator) too.
"""
import json
import subprocess
import sys
import numpy as np
import soundfile as sf

SR = 96000


def truth(x):
    """A made-up but realistic asymmetric saturation curve (the 'unknown')."""
    # asymmetric tanh with a touch of 2nd-harmonic bias + soft makeup
    return np.tanh(1.8 * x + 0.15) - np.tanh(0.15)


def main():
    # 1. make the probe
    subprocess.run([sys.executable, "tools/make_probe.py", "/tmp/probe.wav"], check=True)
    probe, sr = sf.read("/tmp/probe.wav")

    # 2. push it through the known device, add a small latency to test alignment
    latency = 137
    out = truth(probe)
    out = np.concatenate([np.zeros(latency), out])[: len(probe)]
    # add a hair of noise to mimic a real render
    out += np.random.default_rng(0).normal(0, 1e-4, len(out))
    sf.write("/tmp/captured.wav", out.astype(np.float32), SR, subtype="FLOAT")

    # 3. extract
    subprocess.run([sys.executable, "tools/extract_profile.py",
                    "/tmp/probe.wav", "/tmp/captured.wav", "/tmp/profile.json",
                    "--name", "selftest"], check=True)

    # 4. compare reconstruction to ground truth over the audible input range
    prof = json.load(open("/tmp/profile.json"))
    lut = np.array(prof["lut"])
    rng = prof["lut_range"]
    centres = np.linspace(-rng, rng, len(lut))

    test_x = np.linspace(-1.0, 1.0, 400)
    recon = np.interp(test_x, centres, lut)
    true = truth(test_x)
    err = recon - true
    rms = np.sqrt(np.mean(err ** 2))
    peak = np.max(np.abs(err))

    print("\n=== SELF-TEST RESULT ===")
    print(f"reconstruction RMS error : {rms:.5f}")
    print(f"reconstruction peak error: {peak:.5f}")
    for xv in (-1.0, -0.5, 0.0, 0.5, 1.0):
        print(f"  x={xv:+.2f}  true={truth(np.array([xv]))[0]:+.4f}  "
              f"recon={np.interp(xv, centres, lut):+.4f}")

    ok = rms < 0.01 and peak < 0.03
    print("\nPASS" if ok else "\nFAIL")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
