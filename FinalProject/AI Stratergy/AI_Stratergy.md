# AI Strategy: Convolution Reverb Plugin

- **Primary goal**: ship a stable JUCE convolution reverb plugin that loads IRs, performs frequency-domain convolution with overlap-add, supports 44.1/48 kHz and typical block sizes (512/1024), and exposes basic controls (dry/wet, output trim, pre-delay if time permits).

- **Architecture**:
  - `ConvolutionReverbAudioProcessor` owns DSP objects; `ConvolutionReverbAudioProcessorEditor` handles UI.
  - DSP helpers: `ConvolutionEngineBase` (per-channel engine), `FFTProcessor` (wraps `juce::dsp::FFT`), `IRLoader` (file read + preprocessing), `ParameterManager` (APVTS + smoothing).
  - RAII for all buffers; no allocations/locks in the audio thread.

- **DSP path (per block, per channel)**:
  1) Copy input block, zero-pad to `fftSize >= 2 * blockSize`.
  2) Forward FFT.
  3) Complex multiply with precomputed IR spectrum.
  4) Inverse FFT.
  5) Overlap-add tail, apply dry/wet mix, write to output; shift/update overlap buffer.

- **IR handling**:
  - GUI thread triggers file chooser; background worker loads WAV/AIFF via `AudioFormatReader`.
  - Normalize/optionally trim silence; mono or stereo aware (reuse mono IR for both channels if needed).
  - Precompute IR FFT (partition later if time) and store interleaved real/imag; atomic, lock-free swap into engine.

- **Threading model**:
  - Audio thread: deterministic processing only (FFT, multiply, IFFT, OLA, parameter smoothing).
  - Background thread: IR load + FFT/preprocessing, then atomic pointer swap.
  - Message thread: UI, meters, file chooser.

- **Parameters & smoothing**:
  - APVTS params: `dryWet`, `outputTrim`, optional `preDelayMs`.
  - Per-block smoothing (e.g., `juce::dsp::SmoothedValue`) to avoid zips.
  - Dry/wet mix applied after OLA per channel.

- **Buffers & sizing**:
  - Choose `fftOrder` so `fftSize` is power-of-two; start with 2048 for 512/1024 host blocks.
  - Buffers: time-domain input/output, frequency-domain temp, IR spectrum, overlap-add per channel.
  - Clear buffers in `prepareToPlay`; reuse across blocks.

- **Performance/robustness**:
  - Use `ScopedNoDenormals`; avoid `std::complex` temporaries in hot loops.
  - Validate IR length vs. fftSize; guard against NaN/Inf; handle silence (output should decay to zero).
  - Target latency within 10–20 ms; watch CPU with profiler if available.

- **GUI essentials**:
  - File chooser for IR, dry/wet slider, output trim, simple meters (using `juce_audio_utils`).
  - Keep UI responsive by offloading IR processing to background thread.

- **Testing plan**:
  - Offline harness: feed an impulse, expect output equals IR (non-partitioned); feed silence, ensure decay to zero; check for denorms/NaNs.
  - Manual host test for basic playback/mono routing.

- **Stretch improvements (BEST tier)**:
  - Partitioned convolution (small partitions for early reflections, larger for tail).
  - SIMD/vectorization; consider vDSP/FFTW if allowed.
  - Lock-free ring buffers for background-to-audio handoff if more data types are shared.

- **Delivery order**:
  1) Wire basic DSP path (FFT → multiply → IFFT → OLA) for one IR, per channel.
  2) Add IR loading thread + atomic swap.
  3) Add parameters + smoothing + GUI controls.
  4) Run offline impulse/silence tests; then host check.
