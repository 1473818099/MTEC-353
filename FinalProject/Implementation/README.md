# Convolution_Reverb JUCE Plugin

This folder contains a ready-to-build JUCE convolution reverb plugin with a custom FFT/OLA engine.

## Build
1) Make sure you have a JUCE checkout. Point `JUCE_DIR` at the folder that contains `JUCEConfig.cmake`.
2) Configure and build:
   ```
   cmake -B build -S Implementation/Convolution_Reverb -DJUCE_DIR=/path/to/JUCE
   cmake --build build
   ```
3) The plugin targets AU and VST3 by default (`COPY_PLUGIN_AFTER_BUILD` is enabled).

## Features
- Custom FFT-based convolution engine with overlap-add.
- Background IR loading with a file chooser.
- Dry/wet mix and output trim parameters (smoothed).
- Mono IR loading; stereo processing with per-channel engines.

## Usage
- Open the plugin UI and click **Load IR** to pick a WAV/AIFF impulse response.
- Adjust **Dry/Wet** and **Trim** as needed.

## Notes
- The loader assumes the IR sample rate matches the session. Add resampling if you need cross-rate support.
- FFT size adapts to `blockSize + irLength` to avoid time-aliasing. Very long IRs may increase CPU/latency.
