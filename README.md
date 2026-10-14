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
  **Shape pads**: select three of the curve's points and two pads appear on the graph, for the middle one. On the
  Frequency pad, across moves it between the outer two, and up or down makes it higher in one ear than the other. On the
  Gain pad, up or down is its gain, and across makes it louder in the left ear or the right. Double-click a pad to make
  the ears the same again. A point is still one point, on the curve as drawn; each ear's curve goes through its own
  version of it, half the difference each way, and is drawn faintly (blue for left, orange for right).
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
  right are its own volume, a speed (0.5 to 64 bursts a second, 2.5 to start), an attack (how long each burst takes to reach full level: 0 to 200 ms, 4 ms to start; longer is softer), a release (how long each burst takes to
  fade out: 0 ms, which leaves just a tick, up to 3 s; 1.5 s to start), a depth (each row plays that many times before the next row does, a level louder each time),
  repeats (how many times the row plays at each of those levels before going up one), On each (Dot, Row or Cycle: what the depth and
  repeats work on. Dot takes each dot through every level before the next dot; Row, the default, each row; Cycle plays every dot at one level, then every dot at the next), and a floor: how far below the volume the quietest level is (-20 dB to start), with the rest climbing evenly from there to the volume. On the graph, each spot (across its pan steps) is a row. Hiding it stops the sound.
- **Calibration, Drag mode** (the panel's third mode): like the grid, but instead of fixed rows and columns there's
  one square of dots, 2 x 2 or 3 x 3, that you put where you like. Across the field is from your left ear to your
  right, and up it is the low cut, from none at the bottom (20 Hz) to 20 kHz at the top. Drag the square to move it
  (or click elsewhere on the field to bring it there; the arrow keys nudge it), and drag its bottom right corner to
  resize it about its centre. Its dots play in reading order.
- **Calibration, Guided mode** (experimental; the panel's fourth mode): a search that gives you one change to the
  curve at a time, and judges it on a whole arrangement of sounds: left and right, low and high, far and near, all
  at once. (It never plays one of those by itself: a change that straightens the height can squash the depth, and
  you'd not hear it.)
  - **Before you start**, choose the high-frequency bias: how much the curve's points bunch towards the top. A point
    a fraction t of the way along is 1 - (1 - t)^bias of the way from 20 Hz to 20 kHz, in octaves. 1 is evenly spaced
    (20, 112, 632, 3.6k, 20k); 1.5, the default, is 20, 225, 1.7k, 8.4k, 20k; 2 is more so. Play then redraws your
    curve through those 5 points (each ear's curve as close as 5 points allow; it's one undo step). A search keeps
    its bias; Start over ends it, so you can choose again.
  - **Each turn plays a box of sounds**: two layers across (pan: left and right), two up (the low cut: the grid's
    bottom and top rows', about 112 Hz and 3.6 kHz) and two deep (level: the floor, and the volume), so its eight
    corners. It's read like a book, round and round: the near side first, along its top row from left to right and
    then its bottom row, and then the far side the same way; each sound 2, 4 or 8 times (4 to start) before the
    next. That's the order every turn. A drawing of the box shows where each sound should seem to be (what to aim
    for, not a measurement): what's far is smaller, and the one that's playing lights up and is named under it
    ("High, left, near").
  - **One thing to listen for**, every turn: "Make the whole arrangement larger and more consistent. Listen for
    compression, collapse, or distortion anywhere in the arrangement."
  - **Listen first**: the first time the box comes up, and the first time it has twelve sounds, it plays once
    through before the pad does anything.
  - **Then drag the pad**. It makes two changes at once, named under the instructions, by up to the range (+/-30 dB
    to start) each way from the middle, which is no change. You hear it, and see it on the graph. **Hear start**
    plays the setting the turn began with, without losing where the dot is.
  - **Let go**, and it all plays once through with your choice. Then say whether it's **Better**, or **Skip** it.
  - **A better one is checked** twice, on the whole arrangement both times: the same sounds in another order (down
    each side's columns, not along its rows), and then with more positions (a layer between two of the box's sides,
    where the turn hadn't one). Each check plays
    with the starting setting (BEFORE), then your change (AFTER), until you answer Better, Same or Worse. It's kept
    only if it's worse on neither. A turn is one undo step; a change that isn't kept is undone.

  What comes next depends on what happened. A kept change at the pad's edge gets the same pad and sounds again,
  recentred, to go further. Any other kept change gets a pad whose across is that change (left undoes it, right goes
  on) and whose up is whichever change has been tried least, with the first taken out of it; after two of those
  running, it's back to the list. Anything not kept moves on down the list: neighbouring points together, each point
  with its difference between the ears, and neighbouring differences, from the highest point to the lowest.

  A pass is everything on the list, once. After a pass with nothing kept, the box gets a layer between two of its
  sides: three layers of one dimension (across, up and deep taking it in turn) and so twelve sounds, with the corners
  always there, and still in reading order (a third column, a third row, or a third side between the other two). At first the layer's halfway; after another pass with nothing kept, it's nearer one side or the
  other as well. Only after that does the range halve (30, 15, 7.5, 3.75 dB), and after the smallest, the points
  double (5, 9, 17), keeping the ones there were, and it starts on them at +/-15 dB. Speed, Attack, Release, Volume
  and Floor (the level of the box's far side) are the panel's own sliders. Everything tried, and what
  you answered, is written to a file in `~/Library/Application Support/CabinEQ/Guided calibration` (the Log
  button shows it). The search is `GuidedSearch.h`, which knows nothing of audio; `GuidedCalibrationView.h` joins
  it to the curve and the player.
- **Calibration heights** (the Heights buttons beside the modes): how the low cuts are spaced between 20 Hz and
  20 kHz, wherever they come from a height: the grid's rows, up Drag mode's field, and the guided arrangements.
  **Log**, the default, is even in octaves (three rows: about 112 Hz, 632 Hz and 3.6 kHz). **Hz** is even in Hz
  (5, 10 and 15 kHz: nearly everything's high). **Mel** is even in mels, roughly how pitch is heard, and between the
  two (about 970 Hz, 3.2 kHz and 8.2 kHz). On the graph you put the spots where you like, so it does nothing there.
  **Pin**, beside them, opens a Bottom and a Top frequency (100 Hz and 10 kHz to start) and a switch to pin the
  heights to them: the bottom row is then exactly the bottom frequency and the top row exactly the top one, with the
  rest spaced between as Log, Mel or Hz says (a single row is halfway). Drag mode's field runs from one to the other,
  and so do the guided heights. Unpinned, they're spread between 20 Hz and 20 kHz with room left at both ends.
- **Calibration pan** (the Pan button beside those; it's lit when either isn't the usual): two choices for the grid,
  Drag mode and the guided arrangements. **Spacing** is where positions across go: Even (the default), Centre
  (bunched towards the middle, where small differences are easiest to hear: halfway out becomes a quarter of the way
  out) or Sides (bunched out towards your ears). The ends and the middle stay put, and the dots are still drawn
  evenly. **Law** is how a position becomes a level in each ear: Power (the default: equal power, the middle 3 dB down
  in each ear), Linear (a straight crossfade: the middle is 6 dB down, and sounds quieter) or dB (every step across is
  the same number of dB between the ears, up to 30 dB at the ends, so hard left is still faintly in your right ear).
  On the graph, the law applies but the spacing doesn't: its pan range is your own.
- **Calibration noise**: two sliders at the bottom of the panel, both -4.5 dB an octave to start (double-click one
  for that again). **Slope** is how fast each burst falls away above its lowest frequency (pink noise falls 3).
  **Top line** is how fast the bursts' bottom edges fall from one burst to the next: each burst is turned up or down
  by its lowest frequency to put it on that line. With the two the same, as they start, every burst is that same
  noise cut off lower or higher, at the same level.
- **Calibration on the graph** (the panel's "On graph" mode; the grid is the default): 3 spots (or 2 or 4),
  each playing from its frequency (a sharp low cut) up to 20 kHz. They show on the EQ graph as dashed
  lines with chips along the bottom: drag the lowest or highest spot to spread them out from (or in
  towards) the middle, which stays put, and a middle spot to move them all. Two spots have a grip
  between them to move both. They share a pan range (two thumbs); pan steps play each spot at that
  many positions across it, left to right, before the next spot plays. The one playing lights up.
  Zoom in on the graph to place them, and the bands, precisely.
- **Top bar**: undo/redo (Cmd+Z, Cmd+Shift+Z), the profile's preamp, auto gain (keeps
  loudness the same when you switch the EQ on and off), master volume (-30 to +24 dB, with the EQ
  on or off, with nothing limiting or compressing it: the whole signal path is linear, so a boost past full scale clips at your output), calibration, crossfeed, mono (left and right
  summed, the same in both ears, before the EQ and whether it's on or off; the calibration sounds stay where
  they are), and the EQ's on/off.

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
