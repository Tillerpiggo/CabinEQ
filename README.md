# CabinEQ

A parametric EQ for headphones, as a VST3 plugin and a standalone app. CabinEQ System
loads the VST3 to EQ everything the Mac plays.

## Using it

- **Profiles** (left): one per pair of headphones, or however you like. Click to switch,
  double-click to rename, right-click to duplicate, export or delete. **+** makes a new one
  or imports an Equalizer APO / AutoEQ `ParametricEQ.txt` file. You can also drop those
  files on the window.
- **Graph**: click the line to add a band and drag to shape it. Drag bands to move them
  (Cmd for fine, Shift to keep to one axis), scroll or Alt-drag to change their width,
  double-click to turn one off, and right-click for its shape (bell, shelves, cuts),
  which ear it's for, and more. Drag across empty space to select several.
  Delete, arrow keys and Cmd+A work too.
- **Band strip** (bottom): the selected band's exact values. Drag, scroll or double-click
  them to type.
- **Top bar**: undo/redo (Cmd+Z, Cmd+Shift+Z), the profile's preamp, auto gain (keeps
  loudness the same when you switch the EQ on and off), crossfeed, and the EQ's on/off.

## Building

Needs CMake, the Command Line Tools, and JUCE 8.0.4 or newer at `~/JUCE`:

```
git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git ~/JUCE
./update.sh --no-pull
```

`update.sh` builds the VST3, runs the tests and a host check, installs it to
`~/Library/Audio/Plug-Ins/VST3`, keeps the previous version in `build/previous`, and restarts
CabinEQ System if it's running. Without `--no-pull` it pulls from GitHub first.

Tests on their own:

```
cmake --build build --target CabinEQ_Tests CabinEQ_HostCheck
build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests
build/CabinEQ_HostCheck_artefacts/Release/CabinEQ_HostCheck build/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3
build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests --snapshot ui.png   # renders the UI
```

## How it fits together

- `CabinEqAudioProcessor`: the plugin. Profiles live in its parameters' ValueTree, which
  is the one source of truth; the processor listens to it and hands the selected profile
  to the audio path, so UI edits, undo and loading state all work the same way.
- `CabinEqProfile` / `CabinEqProfileManager`: read and edit profiles in that tree, and
  migrate state from older versions (a backup of the old state is saved to
  `~/Library/Application Support/CabinEQ/Backups` first).
- `PlaybackManager`: the audio path, which is `FilterChain` (a preallocated, smoothed pool of
  biquads), then `CrossfeedProcessor`, then gain, with a crossfaded bypass.
- `FilterDesign`: the biquad coefficients, shared by the audio and `BandEqCurve`, so the graph
  draws exactly what you hear.
- UI: `CabinEqPage` holds `ProfileList`, `CabinPeqGraph` and `BandInspector`, styled by
  `CabinEqLookAndFeel` and `Theme`.
