#!/bin/zsh
# Makes dist/CabinEQ-System-<version>.dmg: CabinEQ System, with CabinEQ built into it, for anyone's Mac.
#
#   ./package.sh                 build, test, sign, notarize, staple, and check it the way Gatekeeper will
#   ./package.sh --no-notarize   everything but notarizing (the result warns on other Macs until it is)
#
# Needs your Developer ID Application certificate in the keychain, and, to notarize, your
# App Store Connect login saved once as the "CabinEQ" profile:
#   xcrun notarytool store-credentials "CabinEQ" --apple-id <your Apple ID> --team-id E88BK26E9L
# SIGNING_IDENTITY and NOTARY_PROFILE override those. The DMG window needs dmgbuild (pip install dmgbuild).
set -euo pipefail
cd "${0:A:h}"

identity="${SIGNING_IDENTITY:-Developer ID Application: Tyler Gee (E88BK26E9L)}"
profile="${NOTARY_PROFILE:-CabinEQ}"
notarize=true
[[ "${1:-}" == "--no-notarize" ]] && notarize=false

version=$(sed -nE 's/^project\(CabinEQSystem VERSION ([0-9.]+).*/\1/p' SystemAudioTap/CMakeLists.txt)
dist="$PWD/dist"
work="$dist/work"
app="$work/CabinEQ System.app"
dmg="$dist/CabinEQ-System-$version.dmg"

step() { print -P "\n%F{magenta}==>%f %B$1%b" }
fail() { print -P "%F{red}$1%f"; exit 1 }

# Check everything we need is here before spending ten minutes building
security find-identity -v -p codesigning | grep -q "$identity" || fail "Couldn't find the signing identity \"$identity\" in the keychain."
python3 -c "import dmgbuild" 2>/dev/null || fail "The DMG needs dmgbuild: python3 -m pip install --user dmgbuild"
if $notarize && ! xcrun notarytool history --keychain-profile "$profile" > /dev/null 2>&1; then
    fail "There's no notarization login saved as \"$profile\". Save it with:
  xcrun notarytool store-credentials \"$profile\" --apple-id <your Apple ID> --team-id E88BK26E9L
or run ./package.sh --no-notarize"
fi

rm -rf "$work" && mkdir -p "$work"

step "Building CabinEQ $version for Apple silicon and Intel"
cmake -S . -B build-dist -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 > /dev/null
cmake --build build-dist --target CabinEQ_VST3 CabinEQ_Tests CabinEQ_HostCheck -j 8 > "$work/build-plugin.log" 2>&1 \
    || { tail -30 "$work/build-plugin.log"; fail "The plugin didn't build"; }

step "Testing it"
build-dist/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests > "$work/tests.log" 2>&1 \
    || { tail -20 "$work/tests.log"; fail "Tests failed"; }
build-dist/CabinEQ_HostCheck_artefacts/Release/CabinEQ_HostCheck build-dist/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3 > "$work/hostcheck.log" 2>&1 \
    || { cat "$work/hostcheck.log"; fail "The host check failed"; }
echo "Tests and host check passed"

step "Building CabinEQ System"
# Signed ad hoc here; the real signature goes on once the plugin's inside it
cmake -S SystemAudioTap -B SystemAudioTap/build-dist -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
      -DCABINEQ_SYSTEM_SIGNING_IDENTITY=- > /dev/null
cmake --build SystemAudioTap/build-dist -j 8 > "$work/build-app.log" 2>&1 \
    || { tail -30 "$work/build-app.log"; fail "CabinEQ System didn't build"; }

step "Putting CabinEQ inside it"
ditto "SystemAudioTap/build-dist/CabinEQSystem_artefacts/Release/CabinEQ System.app" "$app"
mkdir -p "$app/Contents/PlugIns"
ditto build-dist/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3 "$app/Contents/PlugIns/CabinEQ.vst3"
xattr -cr "$app" # Finder info and the like break signatures

# The app can only load a plugin that runs on every macOS the app does
min_macos() { vtool -show-build "$1" | awk '/minos/ { print $2 }' | sort -V | tail -1 }
app_min=$(min_macos "$app/Contents/MacOS/CabinEQ System")
plugin_min=$(min_macos "$app/Contents/PlugIns/CabinEQ.vst3/Contents/MacOS/CabinEQ")
[[ "$(printf '%s\n' "$plugin_min" "$app_min" | sort -V | tail -1)" == "$app_min" ]] \
    || fail "The plugin needs macOS $plugin_min, but the app runs on $app_min: it wouldn't load on older Macs"
echo "The app runs on macOS $app_min and later, and the plugin on $plugin_min and later"

step "Signing with $identity"
# Inside out: the plugin, then the app around it, both with the hardened runtime notarizing needs
codesign --force --timestamp --options runtime --sign "$identity" "$app/Contents/PlugIns/CabinEQ.vst3"
codesign --force --timestamp --options runtime --entitlements packaging/CabinEQSystem.entitlements \
         --sign "$identity" "$app"
codesign --verify --deep --strict --verbose=1 "$app"

notarize_and_staple() {
    local item="$1" submission="$2"
    xcrun notarytool submit "$submission" --keychain-profile "$profile" --wait --output-format json > "$work/notary.json" || true
    local id=$(python3 -c "import json; print(json.load(open('$work/notary.json')).get('id', ''))")
    local result=$(python3 -c "import json; print(json.load(open('$work/notary.json')).get('status', ''))")
    if [[ "$result" != "Accepted" ]]; then
        cat "$work/notary.json"
        [[ -n "$id" ]] && xcrun notarytool log "$id" --keychain-profile "$profile"
        fail "Apple didn't accept ${item:t} for notarizing"
    fi
    xcrun stapler staple "$item"
}

if $notarize; then
    step "Notarizing the app (this usually takes a few minutes)"
    ditto -c -k --keepParent "$app" "$work/CabinEQ System.zip"
    notarize_and_staple "$app" "$work/CabinEQ System.zip"
fi

step "Making the disk image"
python3 packaging/make-dmg-background.py "$work"
tiffutil -cathidpicheck "$work/background.png" "$work/background@2x.png" -out "$work/background.tiff" > /dev/null 2>&1
rm -f "$dmg"
python3 -m dmgbuild -s packaging/dmg-settings.py \
    -D app="$app" -D background="$work/background.tiff" -D icon="$app/Contents/Resources/AppIcon.icns" \
    "CabinEQ System" "$dmg" > /dev/null
codesign --force --timestamp --sign "$identity" "$dmg"

if $notarize; then
    step "Notarizing the disk image"
    notarize_and_staple "$dmg" "$dmg"
fi

step "Checking it the way a Mac that's never seen it will"
codesign --verify --deep --strict "$app" && echo "The app's signature is valid"
if $notarize; then
    spctl --assess --type execute --verbose=2 "$app" 2>&1
    spctl --assess --type open --context context:primary-signature --verbose=2 "$dmg" 2>&1
    xcrun stapler validate "$dmg"
else
    echo "Not notarized, so Gatekeeper will warn on other Macs until it is"
fi

(cd "$dist" && shasum -a 256 "${dmg:t}" > "${dmg:t}.sha256")
print -P "\n%F{green}Made $dmg%f ($(du -h "$dmg" | cut -f1))"
