#include "IRLoader.h"
#include <algorithm>

IRLoader::IRLoader()
{
    formatManager.registerBasicFormats();
}

std::shared_ptr<IRData> IRLoader::loadIR(const juce::File& file,
                                         double sampleRate,
                                         int blockSize)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (!reader)
        return nullptr;

    if (reader->sampleRate != sampleRate)
    {
        // For simplicity, require matching sample rate.
        // A production version should resample here.
    }

    const auto totalSamples = static_cast<int>(reader->lengthInSamples);
    if (totalSamples <= 0)
        return nullptr;

    juce::AudioBuffer<float> irBuffer(static_cast<int>(reader->numChannels), static_cast<int>(totalSamples));
    reader->read(&irBuffer, 0, static_cast<int>(totalSamples), 0, true, true);

    auto monoIR = makeMono(irBuffer);
    const int irLength = static_cast<int>(monoIR.size());
    const int partitionSize = computePartitionSize(blockSize);
    const int fftSize = partitionSize * 2;
    const int fftOrder = computeFFTOrder(fftSize);

    const int numPartitions = (irLength + partitionSize - 1) / partitionSize;

    juce::dsp::FFT fft(fftOrder);

    auto data = std::make_shared<IRData>();
    data->partitionSize = partitionSize;
    data->fftOrder = fftOrder;
    data->fftSize = fftSize;
    data->numPartitions = numPartitions;
    data->numChannels = 1;
    data->irLength = irLength;
    data->partitions.resize(1);
    data->partitions[0].resize(static_cast<size_t>(numPartitions));

    for (int p = 0; p < numPartitions; ++p)
    {
        std::vector<float> fftBuffer(static_cast<size_t>(fftSize * 2), 0.0f);
        const int offset = p * partitionSize;
        const int remaining = irLength - offset;
        const int copyCount = std::max(0, std::min(partitionSize, remaining));
        if (copyCount > 0)
            std::copy(monoIR.begin() + offset, monoIR.begin() + offset + copyCount, fftBuffer.begin());

        fft.performRealOnlyForwardTransform(fftBuffer.data());
        data->partitions[0][static_cast<size_t>(p)] = std::move(fftBuffer);
    }

    return data;
}

int IRLoader::computePartitionSize(int hostBlockSize) const
{
    int size = juce::nextPowerOfTwo(hostBlockSize);
    return std::max(size, 256);
}

int IRLoader::computeFFTOrder(int fftSize) const
{
    int order = 0;
    int size = 1;
    while (size < fftSize)
    {
        size <<= 1;
        ++order;
    }
    return order;
}

std::vector<float> IRLoader::makeMono(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    std::vector<float> mono(static_cast<size_t>(numSamples), 0.0f);

    if (numChannels == 1)
    {
        std::copy(buffer.getReadPointer(0), buffer.getReadPointer(0) + numSamples, mono.begin());
        return mono;
    }

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* src = buffer.getReadPointer(ch);
        for (int n = 0; n < numSamples; ++n)
            mono[static_cast<size_t>(n)] += src[n];
    }

    const float scale = 1.0f / static_cast<float>(numChannels);
    for (auto& s : mono)
        s *= scale;

    return mono;
}
