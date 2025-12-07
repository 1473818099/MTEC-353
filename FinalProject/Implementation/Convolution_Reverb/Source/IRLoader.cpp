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

    const int64 totalSamples = reader->lengthInSamples;
    if (totalSamples <= 0)
        return nullptr;

    juce::AudioBuffer<float> irBuffer(static_cast<int>(reader->numChannels), static_cast<int>(totalSamples));
    reader->read(&irBuffer, 0, static_cast<int>(totalSamples), 0, true, true);

    auto monoIR = makeMono(irBuffer);
    const int irLength = static_cast<int>(monoIR.size());

    const int fftOrder = computeFFTOrder(blockSize, irLength);
    const int fftSize = 1 << fftOrder;

    std::vector<float> fftBuffer(static_cast<size_t>(fftSize * 2), 0.0f);
    std::copy(monoIR.begin(), monoIR.end(), fftBuffer.begin());

    juce::dsp::FFT fft(fftOrder);
    fft.performRealOnlyForwardTransform(fftBuffer.data());

    auto data = std::make_shared<IRData>();
    data->fftOrder = fftOrder;
    data->fftSize = fftSize;
    data->numChannels = 1;
    data->irLength = irLength;
    data->frequencyData.push_back(std::move(fftBuffer));

    return data;
}

int IRLoader::computeFFTOrder(int hostBlockSize, int irLength) const
{
    const int needed = hostBlockSize + irLength;
    int size = 1;
    int order = 0;
    while (size < needed)
    {
        size <<= 1;
        ++order;
    }
    return std::max(order, 8); // minimum 256-point FFT
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
