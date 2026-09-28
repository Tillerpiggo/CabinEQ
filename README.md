# CabinEQ

A parametric EQ for headphones, as a VST3 plugin and a standalone app. CabinEQ System
loads the VST3 to EQ everything the Mac plays.

## Using it

- **Profiles** (left): one per pair of headphones, or however you like. Click to switch,
  double-click to rename, right-click to duplicate, export or delete. **+** makes a new one
  or imports an Equalizer APO / AutoEQ `ParametricEQ.txt` file. You can also drop those
  files on the window.
- **Graph**: click the 0 dB line to add a band and drag to shape it. Drag bands to move them
  (Cmd for fine), Shift-drag up and down to change their width (Shift works
  mid-drag too, to stop moving and start widening), double-click to
  turn one off, and right-click to delete it. Drag across empty space to select several.
  Delete, arrow keys and Cmd+A work too. Bands outside the view are off screen, not piled at the
  edge. Scroll (or pinch) anywhere on the graph to zoom in on the
  frequencies around the mouse, scroll sideways or Shift-scroll (or drag the frequency axis) to move
  along, and double-click the frequency axis to see 20 Hz to 20 kHz again. The - / + in the top right zooms the view's dB range
  (+/-3 to +/-60 dB), and so does dragging or scrolling on the dB axis; double-click the axis for
  +/-30 dB. It only changes what's shown, not how far bands can go. The band strip sets the shape (bell, shelves, cuts)
  and which ear it's for.
- **Band strip** (bottom): the selected band's exact values. Drag, scroll or double-click
  them to type.
- **Calibration** (the grid button in the top bar): a grid of positions that play pink noise
  bursts in reading order, through the EQ. Columns go from your left ear to your right. Rows set
  a sharp low cut: n rows draw n lines across 20 Hz to 20 kHz, evenly in octaves, cutting it into
  n + 1 sections, and each row's cut is one of those lines, lowest at the bottom (with 3 rows:
  about 112 Hz, 632 Hz and 3.6 kHz). Click positions to play just
  those: Shift- or Cmd-click, or drag across them, to pick several, and move them with the
  arrow keys (Shift+arrow adds the next one); Play all goes back to every
  one. The rows slider runs up the side of the grid and the columns slider along the bottom. On the
  right are its own volume, a speed (0.5 to 8 bursts a second, 2.5 to start), and a depth: each position plays that many times, quietest first,
  10 dB louder each time. Hiding it stops the sound.
- **Calibration on the graph** (the panel's "On graph" mode): 2 or 3 spots, A, B and C, each with its
  own pan. Each plays from its frequency up to the highest spot's (the highest itself up to 20 kHz),
  with sharp edges. They show on the EQ graph as dashed lines: drag a line to move them all together, keeping
  their spacing, or a chip along the bottom to move one. The one playing lights up. Zoom in on the
  graph to place them, and the bands, precisely.
- **Top bar**: undo/redo (Cmd+Z, Cmd+Shift+Z), the profile's preamp, auto gain (keeps
  loudness the same when you switch the EQ on and off), master volume (-30 to +24 dB, with the EQ
  on or off, and a limiter that stops a boost clipping), calibration, crossfeed, and the EQ's on/off.

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
