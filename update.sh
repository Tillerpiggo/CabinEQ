#!/bin/zsh
# Builds the CabinEQ VST3 from this repo and installs it, so CabinEQ System (and any DAW)
# picks it up. If CabinEQ System is running it gets restarted, since it has the old plugin loaded.
#   ./update.sh             pull the latest from GitHub first
#   ./update.sh --no-pull   build what's in this folder as-is
set -euo pipefail
cd "${0:A:h}"

if [[ "${1:-}" != "--no-pull" ]]; then
    git pull --ff-only
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build build --target CabinEQ_VST3 CabinEQ_Tests CabinEQ_HostCheck -j 8

# Don't replace a working plugin with one that fails its checks
build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests > build/tests.log 2>&1 || { tail -20 build/tests.log; echo "Tests failed, so nothing was installed. See build/tests.log"; exit 1; }
build/CabinEQ_HostCheck_artefacts/Release/CabinEQ_HostCheck build/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3 > build/hostcheck.log 2>&1 || { cat build/hostcheck.log; echo "Host check failed, so nothing was installed."; exit 1; }
echo "Tests and host check passed"

built=build/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3
installed="$HOME/Library/Audio/Plug-Ins/VST3/CabinEQ.vst3"

# Keep the previously installed version in build/previous, in case you need to go back.
rm -rf build/previous
mkdir -p build/previous "${installed:h}"
if [[ -d "$installed" ]]; then
    mv "$installed" build/previous/
fi
ditto "$built" "$installed"
echo "Installed $installed"

pid=$(pgrep -x "CabinEQ System" || true)

if [[ -n "$pid" ]]; then
    executable=$(ps -o comm= -p "$pid")
    osascript -e 'quit app "CabinEQ System"'
    while kill -0 "$pid" 2> /dev/null; do sleep 0.2; done
    open "${executable%/Contents/MacOS/*}"
    echo "Restarted CabinEQ System"
else
    echo "Open CabinEQ System to use it."
fi
