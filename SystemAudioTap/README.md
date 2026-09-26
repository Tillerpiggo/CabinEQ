# SystemAudioTap

Runs CabinEQ (or any VST3/AU) on **all system audio on macOS, with no virtual audio device**.

It uses Core Audio process taps (macOS 14.2+): a tap captures every other app's output and mutes the original while it's being read, then a private aggregate device clocks the tap together with the real output device. Each audio callback sends the tapped audio through the plugin and out to the speakers. Nothing gets installed, and if the app quits or crashes, normal audio comes straight back.

## Run it

Needs CMake and JUCE at `~/JUCE` (or pass `-DJUCE_DIR=...` to CMake).

```bash
./run-test.sh
```

This builds the app and runs its automated test. The test plays 4 s of pink noise with `afplay` and checks that the noise went tap → plugin → speakers, with no feedback and enough real-time headroom. It writes `results/report.txt`, plus `dry.wav` (what the tap heard) and `wet.wav` (what the plugin sent to the speakers).

To use it for real, double-click the app in Finder (you can drag it to Applications), or:

```bash
open build/TapTest_artefacts/Release/TapTest.app
```

It opens the plugin's editor with all system audio going through it. Close the window to stop. If it can't start, for example because the plugin isn't installed, it says why.

```bash
./run-test.sh --plugin /absolute/path/Other.vst3
```

This tests a different plugin (the default is `~/Library/Audio/Plug-Ins/VST3/CabinEQ.vst3`).

## Notes

- The first run shows a "System Audio Recording" permission prompt. Without the permission, the tap only delivers silence. The app is signed ad hoc, so a rebuild can make macOS ask again.
- It follows whichever output device is the default when it starts. It doesn't yet handle switching devices mid-session.
- `Source/SystemAudioTap.mm` has the Core Audio part. `Source/Main.cpp` hosts the plugin and runs the test.
