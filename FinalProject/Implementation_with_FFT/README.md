# Convolution_Reverb JUCE Plugin (JUCE FFT version)

This folder mirrors the main convolution reverb project but keeps FFT/IFFT entirely within `juce::dsp::FFT`.

## Build
1) Point `JUCE_DIR` at the folder that contains `JUCEConfig.cmake`.
2) Configure and build:
   ```
   cmake -B build -S Implementation_with_FFT/Convolution_Reverb -DJUCE_DIR=/path/to/JUCE
   cmake --build build
   ```
3) AU and VST3 are enabled by default (`COPY_PLUGIN_AFTER_BUILD` is on).

## Notes
- The convolution engine defers all FFT/IFFT work to `juce::dsp::FFT`; no custom FFT code is used in this variant.
- Partitioned overlap-add processing, dry/wet mix, and output trim match the main project.***
