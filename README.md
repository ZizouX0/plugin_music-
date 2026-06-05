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
The plugin builds **AU** (macOS only), **VST3**, and a **Standalone** app.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

Artifacts land in `build/Decapitone_artefacts/Release/`.

> Tip: to build against a local JUCE checkout instead of downloading it,
> pass `-DDECAPITONE_JUCE_PATH=/path/to/JUCE`.

### macOS (Apple Silicon — M1/M2/M3)

1. **Install the tools** (one-time):

   ```bash
   xcode-select --install            # Xcode command-line tools
   brew install cmake                # or download CMake from cmake.org
   ```

2. **Build** (native arm64 by default; the AU + VST3 are produced):

   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release -j
   ```

   For a Universal binary that also runs on Intel Macs:
   `cmake -B build -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_BUILD_TYPE=Release`

3. **Install** the formats into your user plug-in folders:

   ```bash
   cp -R "build/Decapitone_artefacts/Release/AU/Decapitone.component" \
         ~/Library/Audio/Plug-Ins/Components/
   cp -R "build/Decapitone_artefacts/Release/VST3/Decapitone.vst3" \
         ~/Library/Audio/Plug-Ins/VST3/
   ```

4. **Validate the AU** (Logic only loads AUs that pass this):

   ```bash
   auval -v aufx Dcp1 Zizu
   ```

   You should see `AU VALIDATION SUCCEEDED`.

5. **Load it**:
   - **Ableton Live** → VST3. In *Live ▸ Settings ▸ Plug-Ins*, make sure
     *Use VST3 Plug-In System Folders* is **On**, then click *Rescan*.
     Decapitone appears in the browser under *Plug-Ins* (group **ZizouAudio**).
     Drop it on an audio track.
   - **Logic Pro / GarageBand** → AU. Logic rescans on launch; if it does not
     appear, open *Logic Pro ▸ Settings ▸ Plug-In Manager* and click *Reset &
     Rescan Selection*. It shows up under **ZizouAudio › Decapitone**.
   - **Reaper / Cubase / Studio One / Bitwig** → VST3.

   Because you built it locally it is **not quarantined**, so Gatekeeper will
   not block it. (If you ever move a *downloaded* build, clear quarantine with
   `xattr -dr com.apple.quarantine /path/to/Decapitone.component`.)

> The AU identifiers are subtype `Dcp1`, manufacturer `Zizu` (set in
> `CMakeLists.txt`). The standalone app is at
> `build/Decapitone_artefacts/Release/Standalone/Decapitone.app`.

### Linux build dependencies

On Debian/Ubuntu, JUCE needs a few dev packages:

```bash
sudo apt-get install -y libasound2-dev libx11-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxcursor-dev libfreetype-dev libfontconfig1-dev libgl1-mesa-dev \
  libcurl4-openssl-dev libwebkit2gtk-4.1-dev
```

## CPU usage

Measured with the bundled headless benchmark (48 kHz, 512-sample blocks, 8×
oversampling, stereo, Release/LTO):

| Path | CPU per instance | ≈ instances / core |
|------|------------------|--------------------|
| Analog styles A/E/N/T/P | **3.5 – 7.4 %** | ~13–28 |
| Capture mode (profile + EQ FIR loaded) | **~9 %** | ~11 |

Still light for a saturator. 8× oversampling (raised from 4× to tame aliasing)
is the main cost; the steady-state block does no heap allocation, so there are
no allocation-driven dropouts.

Build and run it yourself:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCH=ON
cmake --build build --target DecapitoneBench
./build/DecapitoneBench_artefacts/Release/DecapitoneBench [optional profile.json]
```

The same option builds `DecapitoneShot`, which renders the editor to a PNG
headlessly (`./DecapitoneShot ui.png 2`) for quick UI iteration.

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
