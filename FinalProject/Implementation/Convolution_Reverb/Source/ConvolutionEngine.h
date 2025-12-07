#pragma once

#include <atomic>
#include <algorithm>
#include <memory>
#include <vector>
#include <juce_dsp/juce_dsp.h>

struct IRData
{
    int fftOrder = 11;
    int fftSize = 2048;
    int numChannels = 1;
    int irLength = 0;
    std::vector<std::vector<float>> frequencyData; // per-channel interleaved real/imag, length = 2 * fftSize
};

class ConvolutionEngine
{
public:
    ConvolutionEngine();
    void prepare(double sampleRate, int blockSize, int numChannels);
    void reset();

    void setIR(const std::shared_ptr<IRData>& ir);
    void setMix(float wetDry);   // 0..1 wet mix
    void setOutputTrim(float db); // dB trim applied after mix

    void process(juce::AudioBuffer<float>& buffer);

private:
    void ensureFFTOrder(int desiredOrder);
    void resizeBuffers(int numChannels);
    void processChannel(int channel, float* samples, int numSamples);

    std::unique_ptr<juce::dsp::FFT> fft;
    int fftOrder = 11;
    int fftSize = 2048;
    int blockSize = 0;

    double sampleRate = 44100.0;
    float wetMix = 0.5f;
    float outputGain = 1.0f;

    std::atomic<std::shared_ptr<IRData>> currentIR{ nullptr };

    std::vector<std::vector<float>> overlapBuffers; // per channel, length fftSize
    std::vector<float> temp;                        // interleaved buffer length 2 * fftSize
};
