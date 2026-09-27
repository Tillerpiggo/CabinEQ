# CabinEQ System

A Mac app that runs CabinEQ (or any VST3/AU) on **all system audio, with no virtual audio device**.

It uses Core Audio process taps (macOS 14.2+): a tap captures every other app's output and mutes the original while it's being read, then a private aggregate device clocks the tap together with the real output device. Each audio callback sends the tapped audio through the plugin and out to the speakers. Nothing gets installed, and if the app quits or crashes, normal audio comes straight back.

## Run it

Needs CMake and JUCE at `~/JUCE` (or pass `-DJUCE_DIR=...` to CMake).

```bash
./run-test.sh
```

This builds the app and runs its automated test. The test plays 4 s of pink noise with `afplay` and checks that the noise went tap → plugin → speakers, with no feedback and enough real-time headroom. It writes `results/report.txt`, plus `dry.wav` (what the tap heard) and `wet.wav` (what the plugin sent to the speakers).

To use it for real, double-click the app in Finder (you can drag it to Applications), or:

```bash
open "build/CabinEQSystem_artefacts/Release/CabinEQ System.app"
```

It opens the plugin's editor with all system audio going through it. Close the window to stop.

The bar along the top always says whether it's working, with meters for what's coming in and going out.
It knows the difference between nothing playing, macOS blocking capture, audio going in but not coming
out, audio stopping (it reconnects by itself), and dropouts. **Setup** opens a checklist, which also
shows by itself on first launch and when something goes wrong:

1. **CabinEQ**: found and loaded, or choose where it is.
2. **Permission**: asks macOS for System Audio Recording, or opens the right page in System Settings
   if it was turned off, and notices as soon as it's turned on.
3. **Output**: it follows the Mac's output (so switching to headphones works), but never plays to a
   virtual device like BlackHole, since nobody would hear it. If the Mac's output is one, it plays to
   your headphones or speakers instead and offers to make them the Mac's output, so the volume keys work.
   You can also pick a device from the menu in the bar.
4. **Other audio processors**: eqMac, Boom, SoundSource or the old CabinEQ standalone app would EQ the
   audio a second time. It offers to quit them.
5. **Check it's working**: plays a short burst of quiet noise from another app and checks it went in and
   came back out.

Each status change is written to `~/Library/Logs/CabinEQ System.log`.
`"CabinEQ System.app/Contents/MacOS/CabinEQ System" --snapshot out.png [setup]` renders the window
without touching the audio.

```bash
./run-test.sh --plugin /absolute/path/Other.vst3
```

This tests a different plugin (the default is `~/Library/Audio/Plug-Ins/VST3/CabinEQ.vst3`).

## Updating CabinEQ

CabinEQ System loads whichever `CabinEQ.vst3` is installed in `~/Library/Audio/Plug-Ins/VST3` each time it opens, so you never rebuild it to get CabinEQ changes. From the repo root, run:

```bash
./update.sh
```

It pulls the latest from GitHub, builds the plugin with CMake (no Xcode needed), installs it, and restarts CabinEQ System if it's running. The previous install is kept in `build/previous/`. Add `--no-pull` to build your local changes as they are.

The build needs CabinEQ's icons and font in `Resources/` at the repo root. If any are missing, it stops and lists them.

## Notes

- The first run shows a "System Audio Recording" permission prompt. Without the permission, the tap only delivers silence. The app is signed ad hoc, so a rebuild can make macOS ask again.
- `Source/SystemAudioTap.mm` has the Core Audio part. `Source/Main.cpp` hosts the plugin and runs the test.
