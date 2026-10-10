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
- **Layers** (`● Bands › ● Curve` in the top bar): two EQs, one after the other. A layer's dot turns it on
  or off, and its name picks it as the one the graph edits (Tab swaps). Both on, they stack: the bands
  play first, then the curve on top. On the graph, the bright line is always what you hear, and the dashed
  line is what the layer you're editing sits on. The curve's points sit on the bright line, so you drag
  the sound itself, and new points come out of the dashed line. Only the layer you're editing shows
  handles. Each layer keeps its own edits whether it's on or not. New profiles start with just the curve;
  imports and older profiles with just the bands. The curve:
  that add up, the EQ follows a smooth curve through points you place (a monotone cubic, so it never
  overshoots between them, and flat past the first and last). Drag a point out of the 0 dB line to
  add one. Anywhere else, drag a point to move it (Cmd for fine), or drag across empty space to
  select several (Shift adds to the selection) and then drag any of them to move them all together.
  Right-click deletes; arrow keys nudge; Delete and Cmd+A work too. Experimental: Shift-drag a point
  up or down to swing the points on one side of it about it, like a hinge (a point at the edge of the
  view follows the mouse, nearer ones move in proportion, and the point itself stays put). A point left
  of the middle of the graph swings the points to its left; one right of the middle, those to its right. The first time you switch, the curve starts out tracing your bands. Both are kept, so
  switching back loses nothing. The curve plays as a minimum-phase FIR filter, redesigned in the
  background as you drag and crossfaded in, so there's no added delay and no clicks.
  **Split L/R** (in the strip under the graph) lets you tweak each ear on top of the curve both
  ears get. Both | L | R (or the B, L and R keys) picks what you're editing: Both edits the shared
  curve, which moves both ears; L or R shows that ear's curve with the shared one dashed underneath,
  and you drag the ear's curve directly (what's stored is how far it is from the shared curve, so
  later edits to Both keep each ear's tweak). Turning Split off keeps the shared curve. Both
  ears share one minimum-phase filter for their average, and each ear's difference from it is a
  short linear-phase filter, so the timing between the ears is untouched (a plain filter per ear
  would shift it where they differ, and move low sounds sideways). That costs about 11 ms of delay
  while split, and differences between the ears are smoothed below about 200 Hz.
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
  right are its own volume, a speed (0.5 to 8 bursts a second, 2.5 to start), an attack (how long each burst takes to reach full level: 0 to 200 ms, 4 ms to start; longer is softer), a release (how long each burst takes to
  fade out: 0 ms, which leaves just a tick, up to 3 s; 1.5 s to start), and a depth: each position plays that many times, quietest first,
  10 dB louder each time. Hiding it stops the sound.
- **Calibration on the graph** (the panel's "On graph" mode; the grid is the default): 3 spots (or 2 or 4),
  each playing from its frequency (a sharp low cut) up to 20 kHz. They show on the EQ graph as dashed
  lines with chips along the bottom: drag the lowest or highest spot to spread them out from (or in
  towards) the middle, which stays put, and a middle spot to move them all. Two spots have a grip
  between them to move both. They share a pan range (two thumbs); pan steps play each spot at that
  many positions across it, left to right, each with its whole depth run. The one playing lights up.
  Zoom in on the graph to place them, and the bands, precisely.
- **Top bar**: undo/redo (Cmd+Z, Cmd+Shift+Z), the profile's preamp, auto gain (keeps
  loudness the same when you switch the EQ on and off), master volume (-30 to +24 dB, with the EQ
  on or off, with nothing limiting or compressing it: the whole signal path is linear, so a boost past full scale clips at your output), calibration, crossfeed, and the EQ's on/off.

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
  biquads), then `CurveFilter`, then `CrossfeedProcessor`, then gain, with a crossfaded bypass.
- `CurveResponse` / `CurveFilter`: curve mode. `CurveResponse` interpolates the points;
  `CurveFilter` designs a minimum-phase FIR from it (cepstral method, about 0.3 s long) on its own
  thread whenever the curve changes, and the audio thread swaps it into a zero-latency
  `juce::dsp::Convolution`, which crossfades from the old one. With no curve it's a no-op. Split
  ears get stereo filters with matched phase (`CurveFilter::designSplit`).
- `FilterDesign`: the biquad coefficients, shared by the audio and `BandEqCurve`, so the graph
  draws exactly what you hear.
- UI: `CabinEqPage` holds `ProfileList`, `CabinPeqGraph` and `BandInspector`, styled by
  `CabinEqLookAndFeel` and `Theme`.

## Making a release

```
./package.sh
```

This makes `dist/CabinEQ-System-<version>.dmg`, which is ready to share: CabinEQ System, with CabinEQ built into it,
for Apple silicon and Intel, on macOS 14.2 or later. It builds and tests everything, and signs the plugin and
app with the Developer ID certificate and the hardened runtime (`packaging/CabinEQSystem.entitlements`). It
notarizes and staples the app and the disk image, and checks them the way Gatekeeper will on a Mac that's never
seen them. The disk image opens to a window with an arrow to drag the app into Applications.

The first time, save your App Store Connect login for notarizing (it needs an app-specific password from
account.apple.com):

```
xcrun notarytool store-credentials "CabinEQ" --apple-id <your Apple ID> --team-id E88BK26E9L
```

It also needs `dmgbuild` (`python3 -m pip install --user dmgbuild`). `./package.sh --no-notarize` does
everything but notarizing. The version comes from `SystemAudioTap/CMakeLists.txt`.

Opened from Applications, the app uses the CabinEQ inside it; its setup checklist offers to install that for
DAWs too. A development build (from `update.sh`) uses the one in `~/Library/Audio/Plug-Ins/VST3`.
