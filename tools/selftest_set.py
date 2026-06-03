#!/usr/bin/env python3
"""
selftest_set.py - validate the multi-point capture SET + interpolation.

We build a synthetic device whose curve depends on Drive (more drive -> harder
clip), capture it at Drive 2/4/6/8/10, merge into a set, then check that the
plugin-side blend at an *uncaptured* Drive (e.g. 5 and 7) reconstructs the true
device curve at that Drive. This proves the Drive knob will track a real device.
"""
import json
import subprocess
import sys
import numpy as np
import soundfile as sf

SR = 96000


def device(x, drive):
    """Known drive-dependent saturation: gain scales with Drive."""
    g = 0.5 + 0.35 * drive            # Drive 2->1.2, 10->4.0
    return np.tanh(g * x + 0.1) - np.tanh(0.1)


def bracket_blend(layers, drive):
    """Mirror the C++ CaptureProfile::lookupBlend logic in Python."""
    ds = [L["d"] for L in layers]
    if drive <= ds[0]:
        return layers[0]["lut"]
    if drive >= ds[-1]:
        return layers[-1]["lut"]
    for k in range(len(layers) - 1):
        if ds[k] <= drive <= ds[k + 1]:
            f = (drive - ds[k]) / (ds[k + 1] - ds[k])
            return layers[k]["lut"] * (1 - f) + layers[k + 1]["lut"] * f
    return layers[-1]["lut"]


def main():
    subprocess.run([sys.executable, "tools/make_probe.py", "/tmp/probe.wav"], check=True)
    probe, _ = sf.read("/tmp/probe.wav")

    captured_drives = [2, 4, 6, 8, 10]
    paths = []
    for d in captured_drives:
        out = device(probe, d)
        out = np.concatenate([np.zeros(53), out])[: len(probe)]   # latency
        cap = f"/tmp/cap_{d}.wav"
        sf.write(cap, out.astype(np.float32), SR, subtype="FLOAT")
        prof = f"/tmp/p_{d}.json"
        subprocess.run([sys.executable, "tools/extract_profile.py",
                        "/tmp/probe.wav", cap, prof,
                        "--drive", str(d), "--name", "synth"], check=True)
        paths.append(prof)

    subprocess.run([sys.executable, "tools/make_capture_set.py",
                    "synth-set", "/tmp/set.json", *paths], check=True)

    s = json.load(open("/tmp/set.json"))
    rng = s["lut_range"]
    layers = [{"d": L["drive"], "lut": np.array(L["lut"])} for L in s["layers"]]
    centres = np.linspace(-rng, rng, len(layers[0]["lut"]))

    print("\n=== SET INTERPOLATION TEST (at uncaptured Drives) ===")
    worst = 0.0
    for d in (3, 5, 7, 9):
        blended = bracket_blend(layers, d)
        test_x = np.linspace(-1.0, 1.0, 400)
        recon = np.interp(test_x, centres, blended)
        true = device(test_x, d)
        rms = np.sqrt(np.mean((recon - true) ** 2))
        worst = max(worst, rms)
        print(f"  Drive={d:2d}  blended-vs-true RMS error = {rms:.4f}")

    print(f"\nworst error: {worst:.4f}")
    ok = worst < 0.03
    print("PASS" if ok else "FAIL")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
