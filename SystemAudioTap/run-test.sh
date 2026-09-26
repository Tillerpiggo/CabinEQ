#!/bin/zsh
# Builds TapTest and runs it.
#   ./run-test.sh                           automated test against CabinEQ.vst3
#   ./run-test.sh --listen                  route all system audio through the plugin until you close its window
#   ./run-test.sh --plugin /abs/path/X.vst3 use a different plugin
set -euo pipefail
cd "${0:A:h}"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build build -j 8

app=build/TapTest_artefacts/Release/TapTest.app
# Sign the whole bundle (the linker only signs the binary), so macOS can remember the permission.
codesign --force --sign - "$app"

out="$PWD/results"
rm -rf "$out" && mkdir -p "$out"

# Launch through LaunchServices rather than as a child of this shell, so macOS treats
# TapTest as its own app when it asks for the System Audio Recording permission.
open -W -n --stdout "$out/log.txt" --stderr "$out/log.txt" "$app" --args --out "$out" "$@"

cat "$out/report.txt"
[[ " $* " == *" --listen "* ]] || grep -q '^RESULT: PASS' "$out/report.txt"
