# Baï-G Sub Bass — v1.0 source project

A lightweight JUCE instrument prototype for deep sub-bass synthesis.

## Included
- 64 factory starting presets across Afrobeat, Amapiano, 808/Trap, club, deep/pure, analog, cinematic and experimental families (Deep, Pure Sine, Analog, 808, Punchy, Afro, Cinematic, Moving)
- MIDI-triggered sine-based sub oscillator with harmonic layers
- Saturation/drive, punch, release, glide, tone, sub-octave blend, output and low-pass cutoff controls
- Photorealistic bass-guitar hero artwork in the interface
- CMake project and GitHub Actions builds for Windows and macOS

## Build
Requirements: CMake 3.22+, a C++17 compiler, and an internet connection for the first JUCE fetch.

```bash
cmake -S . -B build
cmake --build build --config Release
```

Windows: Visual Studio 2022 Build Tools.
macOS: Xcode Command Line Tools.

The GitHub Actions workflow uploads build artifacts after a successful run.

## Important notes
This is an original synthesizer inspired by the general sound-design qualities of professional sub-bass instruments. It does not contain Omnisphere or Logic Pro samples/presets.
The current prototype is a functional starting point, not a finished commercial release. Verify builds in your DAW and test gain staging before production use.


## Sound quality notes
The synth uses a clean fundamental oscillator with controllable harmonic layers, soft saturation,
a low-pass stage, mono-compatible low-frequency output, and parameterized envelopes/glide.
The 64 presets are original starting points intended for Afrobeat, Amapiano, modern 808 and
cinematic bass design. They are not samples or copies of commercial preset libraries.

A production-grade release still needs listening tests across sample rates, DAWs and audio interfaces,
plus CPU, aliasing, MIDI legato, polyphony, preset naming and gain-staging QA. Loudness and perceived
power depend on the arrangement, key, octave, monitoring system and mix; no plugin can guarantee
a universal "world-class" result without those tests.


## World Class Edition prototype update
- 160 selectable preset entries: 64 curated base parameter sets plus systematic tonal variations.
- Additional LFO rate/depth modulation controls.
- Output limiter stage (ceiling target -1 dB) and low-pass filtering.
- Updated interface with live output-level visualization and bass artwork.
- MIDI synth voice with sine/triangle blend, harmonic partials, glide, punch and release.

Still required before a commercial-grade claim: implement true oversampled voice rendering, validated FFT spectrum analyzer,
A/B snapshot management, extensive preset-by-preset sound design/listening, DAW compatibility testing and CPU/latency QA.
The on-screen trace is a live level visualization, not a calibrated FFT spectrum analyzer.


## Interface refresh
The editor now uses the latest approved black / warm-gold / restrained-purple visual direction. The bass-studio header image is embedded as a JUCE binary asset. The screenshot is a visual reference; the real controls are JUCE controls connected to the synthesizer parameters.

## Build status
This package contains the source project, not a precompiled installer. Build VST3 and Standalone with CMake/JUCE or run the included GitHub Actions workflow. The project has not yet been compiled and tested on a Windows DAW in this environment.
