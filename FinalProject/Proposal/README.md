# 1. Project Overview - Convolution Reverb
This is an DSP plugin that can:
- Recreates the acoustics of a real space by applying its impulse response to the audio

Features:
- Full FFT/IFFT Formula coding, overlap buffer 
- DSP plugin
- GUI

Innovation/Interest:
- Learn FFT/DFT in DSP, learn the fundemental theory of Convolution Reverb

# 2. Technical Specification
JUCE Modules: 
- juce_core 
- juce_audio_basics 
- juce_audio_processors 
- juce_audio_devices 
- juce_audio_formats



# Steps

1. Project setup
    - Create a ConvolutionReverb class
    - AudioProcessor holds one instance and calls it in prepareToPlay() and processBlock().

2. Data Structures
    - IR time-domain buffer

    - IR frequency-domain buffer (FFT)

    - FFT engine (juce::dsp::FFT, or another)

    - Temporary block buffer

    - Overlap-add buffer

    - Multiple partitions for partitioned convolution

3. Load IR file
    - Use AudioFormatReader to load a WAV/AIFF IR file.
    - Setup blocksize and fftSize.
    - Split IR into partitions (256-sample partitions)
    - Store them in an array: H_partition[p][k]

4. Pre-compute the FFT of IR
    - Allocate Frequency-domain IR buffer
    - Run FFT on the IR

5. prepareToPlay() Initialization

    Allocate:

    - inputBlockTimeDomain

    - inputBlockFrequencyDomain

    - outputFrequencyDomain

    - ifftTimeBuffer

    - overlapAddBuffer

    Clear all buffers.

6. processBlock(): Main Convolution Loop

    For each incoming audio block (e.g., 512 samples):
    - Copy input audio block
    - Zero-pad to fftSize
    - FFT the input block
    - Frequency-Domain Multiplication
    - IFFT - Convert back to time domain:
    - Overlap-Add

7. Wet/Dry Mix
8. Multichannel Handling
    - Run one convolution instance per channel

9. Optional Optimizations (if have enough time) (Advanced) !!!!!!

    - Partitioned convolution (small partitions for early reflections, larger for tail)
    - SIMD/vectorization
    - Background threads for loading IR
    - vDSP / FFTW for faster FFTs
    - Lock-free ring buffers








# DFT and FFT Theory fundementals

## Taylor Series Formula
The Taylor Series is a mathematical method that represents a smooth function as an infinite polynomial expansion. Instead of working with the original function—which might involve exponentials, trigonometric functions, or other complicated expressions—the Taylor Series approximates it using simple powers of (x−a).
<p align="center">
    <img src="images/1.png" width="200">

## Euler’s Formula
Euler’s formula states:
<p align="center">
    <img src="images/3.png" width="200">

We can prove this directly using the Taylor series expansions of the exponential, cosine, and sine functions.

By using Taylor Series:
<p align="center">
    <img src="images/4.1.png" width="200">

<p align="center">
    <img src="images/4.2.png" width="200">

<p align="center">
    <img src="images/4.3.png" width="200">

We define the Imaginary number i: i^2 = -1 then,
<p align="center">
    <img src="images/4.4.png" width="200">

Even-power terms (real):
<p align="center">
    <img src="images/4.5.png" width="200">

This exactly matches the Taylor series of cos θ.

Odd-power terms (imaginary):

<p align="center">
    <img src="images/4.6.png" width="200">

So the sum becomes: cosθ+i*sinθ

Then e^iθ = cosθ+i*sinθ

## DFT: Discrete Fourier Transform 
The Discrete Fourier Transform (DFT) converts a finite sequence of time-domain samples into an equivalent representation in the frequency domain. It decomposes a signal into a set of discrete frequency components, each with a magnitude and phase.
<p align="center">
    <img src="images/2.png" width="200">

k = frequency index

n = time index

x[n] = the amplitude of the signal at time index 

The output of Descrete Fourier Transform gives us Phasing spectrum and Magnitude Spectrum. 

However, DST needs N^2 computation cost, but FFT makes it into N logN.

## FFT: Fast Fourier Transform
When 𝑁 is a power of 2 (e.g., 8, 16, 1024…), the DFT can be split into two parts:

(1) Even-indexed samples (2) Odd-indexed samples

From this separation, we obtain the key FFT relation:
<p align="center">
    <img src="images/5.png" width="200">

𝐸
[
𝑘
]
E[k] = DFT of the even-indexed samples

𝑂
[
𝑘
]
O[k] = DFT of the odd-indexed samples

The total computation cost becomes:

𝑂
(
𝑁
log
⁡
𝑁
)
O(NlogN)