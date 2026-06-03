#!/usr/bin/env python3
"""
make_capture_set.py - merge several single captures into one capture SET.

A single profile.json clones one Drive setting. To make the plugin's Drive knob
behave like the real device across its whole range, capture the device at a few
Drive points and merge them here. The plugin then blends between the two nearest
curves as you turn Drive.

Capture e.g. Decapitator Style A at Drive 2, 4, 6, 8, 10:

    python3 make_probe.py probe.wav
    # render probe through Decapitator at Drive=2 -> cap2.wav, Drive=4 -> cap4.wav, ...
    python3 extract_profile.py probe.wav cap2.wav  p2.json  --drive 2 --name "A"
    python3 extract_profile.py probe.wav cap4.wav  p4.json  --drive 4 --name "A"
    python3 extract_profile.py probe.wav cap6.wav  p6.json  --drive 6 --name "A"
    python3 extract_profile.py probe.wav cap8.wav  p8.json  --drive 8 --name "A"
    python3 extract_profile.py probe.wav cap10.wav p10.json --drive 10 --name "A"
    python3 make_capture_set.py "Decapitator A" set_A.json p2.json p4.json p6.json p8.json p10.json

Load set_A.json in the plugin (Style C). Drive now sweeps the captured range.
"""
import sys
import json


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)

    set_name = sys.argv[1]
    out_path = sys.argv[2]
    inputs = sys.argv[3:]

    layers = []
    sr = None
    rng = None
    for path in inputs:
        p = json.load(open(path))
        if p.get("format") != "decapitone-capture-1":
            print(f"  skip {path}: not a single capture")
            continue
        sr = sr or p["sr"]
        rng = rng or p["lut_range"]
        layers.append({
            "drive": float(p.get("drive", 5.0)),
            "lut": p["lut"],
            "eq_mag": p.get("eq_mag", []),
        })

    if len(layers) < 2:
        raise SystemExit("Need at least 2 captures to build a set.")

    layers.sort(key=lambda L: L["drive"])

    out = {
        "name": set_name,
        "format": "decapitone-captureset-1",
        "sr": sr,
        "lut_range": rng,
        "layers": layers,
    }
    json.dump(out, open(out_path, "w"))
    drives = ", ".join(f"{L['drive']:g}" for L in layers)
    print(f"Wrote {out_path}")
    print(f"  set name   : {set_name}")
    print(f"  layers     : {len(layers)}  (Drive points: {drives})")


if __name__ == "__main__":
    main()
