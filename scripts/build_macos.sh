#!/usr/bin/env bash
#
# build_macos.sh - build, install, and validate Decapitone on macOS.
#
# Usage:
#   ./scripts/build_macos.sh            # native (Apple Silicon arm64)
#   ./scripts/build_macos.sh universal  # Universal binary (arm64 + x86_64)
#
set -euo pipefail

cd "$(dirname "$0")/.."

ARCHES="arm64"
if [[ "${1:-}" == "universal" ]]; then
  ARCHES="arm64;x86_64"
fi

echo "==> Configuring (arches: $ARCHES)"
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$ARCHES"

echo "==> Building"
cmake --build build --config Release -j"$(sysctl -n hw.ncpu)"

REL="build/Decapitone_artefacts/Release"

echo "==> Installing AU + VST3 into ~/Library/Audio/Plug-Ins"
mkdir -p ~/Library/Audio/Plug-Ins/Components ~/Library/Audio/Plug-Ins/VST3
if [[ -d "$REL/AU/Decapitone.component" ]]; then
  rm -rf ~/Library/Audio/Plug-Ins/Components/Decapitone.component
  cp -R "$REL/AU/Decapitone.component" ~/Library/Audio/Plug-Ins/Components/
  echo "    installed AU"
fi
if [[ -d "$REL/VST3/Decapitone.vst3" ]]; then
  rm -rf ~/Library/Audio/Plug-Ins/VST3/Decapitone.vst3
  cp -R "$REL/VST3/Decapitone.vst3" ~/Library/Audio/Plug-Ins/VST3/
  echo "    installed VST3"
fi

echo "==> Validating AU with auval"
auval -v aufx Dcp1 Zizu || {
  echo "auval reported a problem (see above)."; exit 1; }

echo
echo "Done. Open Logic/GarageBand (AU) or your VST3 host."
echo "If Logic doesn't show it: Settings > Plug-In Manager > Reset & Rescan Selection."
echo "It appears under: ZizouAudio > Decapitone"
