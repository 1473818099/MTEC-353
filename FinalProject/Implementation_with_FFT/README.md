# Convolution_Reverb_0001 — User Manual

## Overview
- Partitioned convolution reverb audio plugin (AU/VST3) using JUCE FFT/IFFT.
- Load your own impulse responses to add space, with smooth dry/wet and output trim controls.
- Partitioned overlap-add for low latency; mono IRs are supported, processed in stereo.

## Installation
1) Requirements: macOS 11+, C++17 toolchain, CMake 3.15+, JUCE cloned locally (path containing `JUCEConfig.cmake`).
2) Configure from repo root:
   ```
   cmake -B Implementation_with_FFT/build -S Implementation_with_FFT -DJUCE_DIR=/path/to/JUCE -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
   ```
3) Build:
   ```
   cmake --build Implementation_with_FFT/build --config Release
   ```
4) Artifacts:
   - VST3: `Implementation_with_FFT/build/Convolution_Reverb_artefacts/Release/VST3/Convolution_Reverb_0001.vst3`
   - AU: `Implementation_with_FFT/build/Convolution_Reverb_artefacts/Release/AU/Convolution_Reverb_0001.component`
5) Installation: copy the VST3 to `/Library/Audio/Plug-Ins/VST3/` (preferred) and rescan in your host. For AU, copy to `/Library/Audio/Plug-Ins/Components/`. If Gatekeeper blocks, clear quarantine: `sudo xattr -dr com.apple.quarantine <plugin>`.
6) Troubleshooting: ensure Live uses system VST3 folders; delete any older `Convolution_Reverb.vst3` to avoid duplicate IDs; toggle Live’s plug-in scan or delete its plug-in cache and rescan.

## Usage Guide
1) Insert `Convolution_Reverb_0001` on an audio track.
2) Click “Load IR” to choose a mono or stereo WAV/AIFF IR at the host sample rate.
3) Parameters:
   - Dry/Wet (0–1): blend between dry input and convolved output.
   - Output Trim (dB, -24 to +24): gain applied after mixing.
4) Signal flow: input -> partitioned FFT convolution -> wet/dry mix -> output trim.
5) Supported formats: AU, VST3; tested stereo I/O at common sample rates (44.1–192 kHz).

## Examples
- Short room IR: set Dry/Wet around 0.25, Trim 0 dB for natural space.
- Long hall IR: start Dry/Wet 0.5, trim down -6 dB to avoid clipping.
- Creative: load reverse IR, push Dry/Wet to 0.7 for swell effects.

## Known Limitations
- IR sample rate must match host (no resampling implemented).
- Mono IRs are summed if stereo; multichannel beyond stereo not supported.
- No built-in IR browser or presets; relies on file chooser.
- No automation smoothing beyond basic parameter smoothing (20 ms).
