# DESIGN — Convolution_Reverb_0001

## 1. Architecture Overview
- **High-level**: JUCE plug-in (AudioProcessor/Editor) wrapping a partitioned convolution engine. IRs are loaded asynchronously, partitioned, and transformed once; audio thread performs FFT/accumulate/IFFT per block.
- **Threads**: Audio thread runs `processBlock` and `ConvolutionEngine::process`; GUI thread handles UI + async IR file chooser; background std::async parses and partitions IRs, then atomically swaps the IR.
- **Class roles**:
  - `Convolution_ReverbAudioProcessor`: lifecycle, parameters, smoothing, IR load trigger.
  - `Convolution_ReverbAudioProcessorEditor`: UI (load button, two knobs).
  - `IRLoader`: reads IR file, converts to mono, partitions, precomputes spectra.
  - `ConvolutionEngine`: real-time partitioned overlap-add convolution using `juce::dsp::FFT`.
- **Data flow**: Host buffer -> copy dry -> chunked FFT -> frequency-domain multiply-add with IR partitions -> IFFT -> overlap add -> dry/wet mix -> output trim.

## 2. DSP Implementation
- **Partitioned convolution**: Input split into blocks of `partitionSize` (next power-of-two ≥ host block, min 256). FFT size = 2 * partitionSize.
- **Frequency-domain multiply**: For each IR partition p, use ring-buffered input spectra and precomputed IR spectra; accumulate complex products per bin.
- **IFFT and overlap**: Inverse FFT is unscaled; we scale by 1/fftSize. The tail beyond `chunkSize` is stored in an overlap buffer for the next block.
- **Mono IR**: Stereo IRs are summed to mono; convolution is per-output-channel using the nearest IR channel.
- **Latency**: At least one partition; latency roughly one partition (≥256 samples). No explicit latency report to host (could be added).

## 3. Key Technical Decisions
- **Partitioned overlap-add** vs direct convolution: chosen for real-time efficiency; trades latency for O(N log N) per block.
- **JUCE FFT** vs custom FFT: rely on `juce::dsp::FFT` to avoid third-party deps and keep portability.
- **Mono IR processing** vs full multichannel: simplifies memory/CPU; acceptable for assignment scope.
- **Async IR loading** vs blocking: prevents UI/audio stalls; uses atomic pointer swap for thread safety.
- **Smoothing parameters** vs raw values: 20 ms smoothing on dry/wet and output trim to avoid zipper noise without heavy CPU.

## 4. Performance Analysis
- CPU: dominated by FFTs and bin-wise complex multiplies; scales with partitionSize and number of partitions.
- Memory: per-channel overlap buffer (fftSize), per-channel ring buffer of spectra (numPartitions × 2×fftSize floats).
- Optimizations: use precomputed IR spectra; reuse buffers; avoid allocation in audio thread; simple scaling instead of per-sample gain objects.
- Profiling: not instrumented beyond manual inspection; room for SIMD and reduced partition counts for shorter IRs.

## 5. Testing Strategy
- Manual host testing: load various IR lengths (short room, long hall, reverse) and adjust dry/wet and trim; verify wet signal present.
- AU validation: `auval -v aufx CvRv CvRb` (passes).
- Platform: macOS, universal binary (arm64/x86_64). No automated unit tests included.

## 6. Code Walkthroughs
- **IRLoader::loadIR**: Reads file via JUCE, enforces matching sample rate, sums to mono, partitions by `partitionSize`, zero-pads, and FFTs each partition once. Edge cases: zero-length IR returns nullptr; partitions sized to ceil(irLength/partitionSize).
- **ConvolutionEngine::processChunk**: Copies chunk to `tempFreq`, forward FFT, stores spectrum into ring buffer, accumulates frequency products with all IR partitions, inverse FFT, scales, mixes wet/dry, stores tail into overlap. Edge cases: guards null IR; clamps channel index; handles partial final chunk.
- **Parameter smoothing in PluginProcessor**: `SmoothedValue` updated per block, then applied to engine setters before processing; avoids parameter jumps causing clicks.

## Future Improvements
- Report latency to host; add IR resampling; stereo/multichannel IR support; SIMD optimization of bin-wise multiply; preset and IR browser; automated tests with rendered buffers. 
