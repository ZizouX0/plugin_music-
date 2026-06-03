# Matching another saturator exactly — the Capture workflow

The five built-in styles (A/E/N/T/P) are *original* approximations. If you want
Decapitone to sound **identical to a specific device** — Decapitator at a chosen
Style + Drive, a hardware preamp, a tape machine, another plugin — use the
**Capture** engine (Style **C**).

This is the same idea used by amp-capture tech (Kemper, Neural Amp Modeler):
we **measure** the device's input→output behaviour and reproduce it. We do not
copy anyone's code or algorithm — we profile the *sound*.

## Why this works

A saturator like Decapitator is, per Style, a **Wiener–Hammerstein** system:

```
input → [pre-EQ] → [static waveshaper f(x)] → [post-EQ] → output
```

Both the curve `f(x)` and the EQ are fully measurable from audio. Our self-test
reconstructs a known curve to **0.02% RMS error**, so a real capture is just as
faithful.

## Requirements

- Python 3 with `numpy scipy soundfile` (`pip install numpy scipy soundfile`)
- The device you want to capture (e.g. you own Decapitator), in any DAW that can
  render audio offline.

## Steps

1. **Generate the probe signal:**
   ```bash
   python3 tools/make_probe.py probe.wav
   ```
   This writes `probe.wav` (96 kHz) and `probe.wav.meta.json`.

2. **Render it through the target device:**
   - Put `probe.wav` on a track.
   - Insert **only** the device you want to capture (Decapitator). Pick the
     Style and Drive you want to clone. **Bypass everything else.**
   - Make sure the project sample rate is **96 kHz** and there's no extra
     gain/limiting on the master.
   - Export/bounce the **wet** output to `captured.wav` (same length, 96 kHz,
     float or 24-bit).

3. **Extract the profile:**
   ```bash
   python3 tools/extract_profile.py probe.wav captured.wav profile.json --name "Deca A drv5"
   ```
   It auto-aligns latency, then writes `profile.json` (curve + EQ).

4. **Load it in the plugin:**
   - Set **Style = C – Capture**.
   - Click **LOAD CAPTURE** and choose your `profile.json`.
   - The Drive knob now acts as input trim around the captured operating point
     (centre = the level you captured at). Keep Tone/cuts neutral to match the
     source exactly.

Capture as many settings as you like — one `profile.json` per Style/Drive — and
switch between them by loading different files. The loaded profile is saved with
your project.

## Verifying a capture

To prove the maths end-to-end on your machine (no Decapitator needed):

```bash
python3 tools/selftest.py
```

It builds a synthetic device with a known curve, runs the full pipeline, and
reports the reconstruction error (should print **PASS**).

## Limitations (honest notes)

- A capture clones **one** Style/Drive setting. It does not make Decapitator's
  *knobs* move — for that you'd capture several points and interpolate (a future
  enhancement).
- Time-variant behaviour (e.g. real tape wow/flutter, bias drift) is not a
  static curve and won't be captured. Decapitator's models are essentially
  static, so this isn't a problem for it.
- The capture is only as clean as your render: no dither pumping, no master-bus
  processing, correct sample rate.
