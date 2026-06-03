# Decapitone

An analog **saturation / distortion** plugin in the spirit of Soundtoys
Decapitator, built with [JUCE](https://juce.com) and C++. Builds as **VST3**
and a **Standalone** app.

It runs five voiced waveshaping models inside 4× oversampling to keep the
harmonics musical instead of aliased, with a tilt tone control, low/high cut
filters, a **Punish** mode for extreme drive, dry/wet mix, auto-gain, and
output trim.

## The five styles

| Style | Inspired by | Character |
|-------|-------------|-----------|
| **A** | Ampex 350 tape preamp | Soft, warm, gentle even-harmonic glue |
| **E** | EMI TG12345 console | Punchy, bright, controlled odd grit |
| **N** | Neve 1057 input | Creamy, thick even harmonics |
| **T** | Triode tube | Asymmetric, fat 2nd harmonic |
| **P** | Pentode tube | Aggressive, buzzy odd harmonics |
| **C** | **Capture** | Reproduces a *measured* device (see below) |

## Capture mode — sound exactly like another device

Style **C** plays back a **captured profile** of a real saturator (e.g.
Decapitator at a chosen Style/Drive). You render a probe signal through the
target once, and the toolchain measures its exact transfer curve + EQ. This is
the same approach as amp-capture tech (Kemper / Neural Amp Modeler) — it
profiles the *sound*, copying no code. Full guide: **[CAPTURE.md](CAPTURE.md)**.

```bash
python3 tools/make_probe.py probe.wav        # 1. make the probe
# 2. render probe.wav through the target device -> captured.wav
python3 tools/extract_profile.py probe.wav captured.wav profile.json
python3 tools/selftest.py                    # validate the pipeline (PASS)
```

## Controls

- **Drive** – amount of saturation (0–10)
- **Style** – the analog model (A/E/N/T/P)
- **Tone** – tilt EQ, dark ↔ bright
- **Low Cut / High Cut** – pre/post band-limiting
- **Punish** – opens a far hotter drive range for broken textures
- **Mix** – parallel dry/wet blend
- **Output** – final trim (±24 dB)

## Building

You need CMake ≥ 3.22 and a C++17 compiler. JUCE is fetched automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

Artifacts land in `build/Decapitone_artefacts/Release/` (VST3 + Standalone).

> Tip: to build against a local JUCE checkout instead of downloading it,
> pass `-DDECAPITONE_JUCE_PATH=/path/to/JUCE`.

### Linux build dependencies

On Debian/Ubuntu, JUCE needs a few dev packages:

```bash
sudo apt-get install -y libasound2-dev libx11-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxcursor-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev \
  libcurl4-openssl-dev libwebkit2gtk-4.1-dev
```

## Project layout

```
CMakeLists.txt              # build + JUCE fetch
Source/
  PluginProcessor.{h,cpp}   # DSP chain, parameters, oversampling
  PluginEditor.{h,cpp}      # custom dark UI
  dsp/Saturation.h          # the five analog waveshaper models
```

## License

The code here is yours to use. JUCE itself is under its own license — review
[juce.com/get-juce](https://juce.com/get-juce) before distributing binaries.
