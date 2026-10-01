# di75 VST3

VST3 version similar to the REAPER JSFX compressor `jsfx/di75.jsfx`. A fast stereo compressor (DSP derived from Stillwell 1175, BSD license). 

- High-pass filter: 0–400 Hz (0 = off, bit-transparent)
- Threshold −60–0 dB, Ratio 4–20, Gain (makeup) ±20 dB
- Attack 20–2000 µs, Release 20–1000 ms
- Mono switch (collapses L/R after the makeup gain)
- Light-grey faceplate with white knobs (ring tick dots + value labels),
  input/gain-reduction/output bar meters — green for in/out, red for GR,
  with ~3×/s peak-hold — and a mono
  toggle right of the meters; click-a-label typed value entry (Enter commits,
  Escape cancels)
  
This uses a customized vector-drawn replica of the "GUI 1 White" skin of the Tukan Components Library UI. https://github.com/TukanStudios/TUKAN_STUDIOS_PLUGINS
## Golden test

`src/Di75Dsp.cpp` is pure C++17 with no JUCE dependency, and is verified
against a Python float64 reference model of the JSFX (`scripts/make_ref.py`). The generated vectors live in `scripts/data/`
(5 cases including a bit-transparent null test) and are run by CI on every
push:

```sh
ctest --test-dir build -C Release --output-on-failure
```

Each case prints `name maxdiff=<value>`; the test fails if any case exceeds
1e-4 or a file is unreadable, and prints `golden test OK` on success.

To regenerate the vectors (e.g. after intentionally changing the DSP):

```sh
python vst/scripts/make_ref.py
```

## Release process

Pushing a version tag triggers the `di75 VST3` GitHub Actions workflow,
which builds, runs the golden test, and creates a GitHub Release with a zip
of the VST3. The tag can be `v1.0.0` or just `1.0.0` - both are accepted:

```sh
git tag 1.0.0      # or: git tag 1.0.0
git push origin 1.0.0
```

Pushes and pull requests touching `vst/**` run the same build+test without
releasing. CI uses MSVC + Ninja (`ilammy/msvc-dev-cmd`) rather than naming a Visual Studio generator, so it keeps working as GitHub updates the windows-latest image (which no longer ships VS2022).

## License

- The compiled VST3 binary is GPLv3 (JUCE); see `COPYING`.
- DSP core: Stillwell 1175, BSD — copyright retained in `src/Di75Dsp.cpp`.
- GUI design and remaining code: Dwight Ivany, MIT (same as repo root).

## Building locally (Windows)

Requirements: CMake 3.22+ and Visual Studio 2022 with the "Desktop
development with C++" workload. JUCE 8 is fetced automatically by CMake.
```sh
cmake -B build -S vst -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
Copy that folder to `C:\Program Files\Common Files\VST3` (or point your DAW's VST3 search path at it).

