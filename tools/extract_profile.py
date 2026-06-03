#!/usr/bin/env python3
"""
extract_profile.py - turn a probe + its captured output into a profile.json.

Given the original probe.wav (with its .meta.json) and captured.wav (the probe
rendered through the target device, e.g. Decapitator at a chosen Style/Drive),
this reconstructs:

  * the static transfer curve  f(x)  as a 2048-point lookup table over
    x in [-CURVE_AMP, +CURVE_AMP], plus a small linear extrapolation margin;
  * the linear EQ magnitude (combined pre/post emphasis + tone), exported as a
    513-point magnitude response the plugin can match with a short FIR.

The result is written to profile.json, which the plugin's "Capture" engine
loads to reproduce the captured sound.

Usage:
    python3 extract_profile.py probe.wav captured.wav profile.json [--name "A drv5"]
"""
import sys
import json
import numpy as np
import soundfile as sf

LUT_SIZE   = 2048
LUT_RANGE  = 1.5          # store the curve a bit past full-scale for headroom
EQ_BINS    = 513


def load_mono(path):
    x, sr = sf.read(path, always_2d=False)
    if x.ndim > 1:
        x = x.mean(axis=1)
    return x.astype(np.float64), sr


def find_offset(ref, cap, search):
    """Sample offset that best aligns cap to ref, via cross-correlation."""
    n = min(len(ref), len(cap), search)
    r = ref[:n] - ref[:n].mean()
    c = cap[:n] - cap[:n].mean()
    corr = np.correlate(c, r, mode="full")
    lag = np.argmax(corr) - (n - 1)
    return int(lag)


def extract_curve(x_in, y_out, amp):
    """Bin output by input value to recover the static transfer curve."""
    edges = np.linspace(-LUT_RANGE, LUT_RANGE, LUT_SIZE + 1)
    centres = 0.5 * (edges[:-1] + edges[1:])
    sums = np.zeros(LUT_SIZE)
    counts = np.zeros(LUT_SIZE)

    idx = np.digitize(x_in, edges) - 1
    valid = (idx >= 0) & (idx < LUT_SIZE)
    np.add.at(sums, idx[valid], y_out[valid])
    np.add.at(counts, idx[valid], 1)

    lut = np.full(LUT_SIZE, np.nan)
    nz = counts > 0
    lut[nz] = sums[nz] / counts[nz]

    # Fill unvisited bins (the extremes beyond +-amp) by linear extrapolation
    # from the nearest measured slope, so the curve stays well-defined.
    good = np.where(nz)[0]
    if len(good) < 2:
        raise SystemExit("Not enough data to build the curve - check alignment.")
    lut = np.interp(np.arange(LUT_SIZE), good, lut[good])

    # Light smoothing to suppress measurement noise without rounding the knee.
    k = np.hanning(9); k /= k.sum()
    lut = np.convolve(lut, k, mode="same")
    return centres, lut


def extract_eq(ref_sweep, cap_sweep):
    """Combined linear magnitude response from the (low-level) sweep section."""
    n = min(len(ref_sweep), len(cap_sweep))
    n = 1 << int(np.floor(np.log2(n)))           # power of two for the FFT
    R = np.fft.rfft(ref_sweep[:n])
    C = np.fft.rfft(cap_sweep[:n])
    H = C / (R + 1e-9)
    mag = np.abs(H)
    # Resample magnitude onto EQ_BINS log-ish bins (here: linear FFT bins -> fixed grid)
    src = np.linspace(0, 1, len(mag))
    dst = np.linspace(0, 1, EQ_BINS)
    mag = np.interp(dst, src, mag)
    # Tame extremes; normalise so the mid-band sits near unity.
    mag = np.clip(mag, 1e-3, 1e3)
    mag /= np.median(mag[EQ_BINS // 8: EQ_BINS // 2] + 1e-9)
    return mag


def main():
    if len(sys.argv) < 4:
        print(__doc__); sys.exit(1)
    probe_path, cap_path, out_path = sys.argv[1:4]
    name = "captured"
    if "--name" in sys.argv:
        name = sys.argv[sys.argv.index("--name") + 1]

    with open(probe_path + ".meta.json") as f:
        meta = json.load(f)
    sr = meta["sr"]

    ref, sr_r = load_mono(probe_path)
    cap, sr_c = load_mono(cap_path)
    if sr_r != sr or sr_c != sr:
        raise SystemExit(f"Sample-rate mismatch: probe={sr_r}, capture={sr_c}, expected {sr}. "
                         "Re-render the capture at the probe's sample rate.")

    # Align the capture to the probe (search the first ~1.5 s).
    offset = find_offset(ref, cap, int(1.5 * sr))
    if offset > 0:
        cap = cap[offset:]
    elif offset < 0:
        cap = np.concatenate([np.zeros(-offset), cap])
    print(f"Alignment offset: {offset} samples")

    def section(arr, start_s, len_s):
        a = int(start_s * sr); b = a + int(len_s * sr)
        return arr[a:b]

    # --- static transfer curve ---
    cs = meta["curve_start"]; cl = meta["curve_len"]
    x_in = section(ref, cs, cl)
    y_out = section(cap, cs, cl)
    m = min(len(x_in), len(y_out))
    centres, lut = extract_curve(x_in[:m], y_out[:m], meta["curve_amp"])

    # --- linear EQ ---
    ss = meta["sweep_start"]; sl = meta["sweep_len"]
    eq = extract_eq(section(ref, ss, sl), section(cap, ss, sl))

    # Estimate broadband makeup so the LUT sits near unity for small signals.
    small = (np.abs(centres) < 0.1)
    slope = np.polyfit(centres[small], lut[small], 1)[0] if small.sum() > 2 else 1.0

    profile = {
        "name": name,
        "sr": sr,
        "lut_range": LUT_RANGE,
        "lut": [round(float(v), 6) for v in lut],
        "eq_mag": [round(float(v), 6) for v in eq],
        "small_signal_gain": round(float(slope), 6),
        "format": "decapitone-capture-1",
    }
    with open(out_path, "w") as f:
        json.dump(profile, f)

    print(f"Wrote {out_path}")
    print(f"  curve points : {len(lut)}")
    print(f"  small-signal gain : {slope:.3f}  ({20*np.log10(abs(slope)+1e-9):+.1f} dB)")
    print(f"  curve at x=+1.0 : {np.interp(1.0, centres, lut):+.3f}")
    print(f"  curve at x=-1.0 : {np.interp(-1.0, centres, lut):+.3f}")


if __name__ == "__main__":
    main()
