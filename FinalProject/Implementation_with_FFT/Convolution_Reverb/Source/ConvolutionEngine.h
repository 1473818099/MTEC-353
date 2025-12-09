#pragma once

#include <atomic>
#include <algorithm>
#include <memory>
#include <vector>
#include <juce_dsp/juce_dsp.h>

struct IRData
{
    int partitionSize = 0;
    int fftOrder = 11;
    int fftSize = 2048; // fftSize >= 2 * partitionSize
    int numPartitions = 0;
    int numChannels = 1;
    int irLength = 0;
    // partitions[channel][partitionIndex] -> interleaved real/imag length 2 * fftSize
    std::vector<std::vector<std::vector<float>>> partitions;
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

    int getPartitionSize() const { return partitionSize; }

    void process(juce::AudioBuffer<float>& buffer);

private:
    void ensureFFTOrder(int desiredOrder);
    void resizeBuffers(int numChannels, int numPartitions);
    void processBlockPartitioned(int channel, float* samples, int numSamples);
    void processChunk(int channel, float* samples, int chunkOffset, int chunkSize);

    std::unique_ptr<juce::dsp::FFT> fft;
    int fftOrder = 11;
    int fftSize = 2048;
    int partitionSize = 1024;
    int numPartitions = 0;
    int blockSize = 0;

    double sampleRate = 44100.0;
    float wetMix = 0.5f;
    float outputGain = 1.0f;

    std::shared_ptr<IRData> currentIR{ nullptr };

    std::vector<std::vector<float>> overlapBuffers;              // per channel, length fftSize
    std::vector<std::vector<std::vector<float>>> inputSpectra;   // per channel, ring buffer of input partitions (numPartitions x 2*fftSize)
    std::vector<int> writePositions;                             // per channel

    std::vector<float> tempFreq;      // interleaved buffer length 2 * fftSize
    std::vector<float> accumFreq;     // accumulation buffer length 2 * fftSize
    std::vector<float> dryCopy;       // scratch for dry signal per host block
};
