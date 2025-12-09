#include "ConvolutionEngine.h"

ConvolutionEngine::ConvolutionEngine() = default;

void ConvolutionEngine::prepare(double newSampleRate, int newBlockSize, int numChannels)
{
    sampleRate = newSampleRate;
    blockSize = newBlockSize;
    ensureFFTOrder(11); // default start, may be changed by IR
    resizeBuffers(numChannels, numPartitions);
    reset();
}

void ConvolutionEngine::reset()
{
    for (auto& ch : overlapBuffers)
        std::fill(ch.begin(), ch.end(), 0.0f);
}

void ConvolutionEngine::setIR(const std::shared_ptr<IRData>& ir)
{
    if (!ir)
        return;

    partitionSize = ir->partitionSize;
    numPartitions = ir->numPartitions;
    ensureFFTOrder(ir->fftOrder);
    std::atomic_store_explicit(&currentIR, ir, std::memory_order_release);
    resizeBuffers(ir->numChannels, ir->numPartitions);
    reset();
}

void ConvolutionEngine::setMix(float wetDry)
{
    wetMix = std::clamp(wetDry, 0.0f, 1.0f);
}

void ConvolutionEngine::setOutputTrim(float db)
{
    outputGain = juce::Decibels::decibelsToGain(db);
}

void ConvolutionEngine::process(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals guard;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    dryCopy.resize(static_cast<size_t>(numSamples));

    resizeBuffers(numChannels, numPartitions);

    for (int ch = 0; ch < numChannels; ++ch)
        processBlockPartitioned(ch, buffer.getWritePointer(ch), numSamples);
}

void ConvolutionEngine::ensureFFTOrder(int desiredOrder)
{
    if (desiredOrder == fftOrder && fft)
        return;

    fftOrder = desiredOrder;
    fftSize = 1 << fftOrder;
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    tempFreq.resize(static_cast<size_t>(fftSize * 2), 0.0f);
    accumFreq.resize(static_cast<size_t>(fftSize * 2), 0.0f);

    for (auto& ch : overlapBuffers)
        ch.resize(static_cast<size_t>(fftSize), 0.0f);
}

void ConvolutionEngine::resizeBuffers(int numChannels, int partitions)
{
    if (numChannels <= 0)
        return;

    overlapBuffers.resize(static_cast<size_t>(numChannels));
    for (auto& ch : overlapBuffers)
        ch.resize(static_cast<size_t>(fftSize), 0.0f);

    inputSpectra.resize(static_cast<size_t>(numChannels));
    writePositions.assign(static_cast<size_t>(numChannels), 0);
    for (auto& channelBuffer : inputSpectra)
    {
        channelBuffer.resize(static_cast<size_t>(std::max(1, partitions)));
        for (auto& spectrum : channelBuffer)
            spectrum.assign(static_cast<size_t>(fftSize * 2), 0.0f);
    }
}

void ConvolutionEngine::processBlockPartitioned(int channel, float* samples, int numSamples)
{
    auto ir = std::atomic_load_explicit(&currentIR, std::memory_order_acquire);
    if (!ir || ir->partitions.empty())
        return;

    // Keep a copy of the dry input to avoid overwriting while mixing.
    std::copy(samples, samples + numSamples, dryCopy.begin());

    int processed = 0;
    while (processed < numSamples)
    {
        const int chunkSize = std::min(partitionSize, numSamples - processed);
        processChunk(channel, samples, processed, chunkSize);
        processed += chunkSize;
    }
}

void ConvolutionEngine::processChunk(int channel, float* samples, int chunkOffset, int chunkSize)
{
    auto ir = std::atomic_load_explicit(&currentIR, std::memory_order_acquire);
    if (!ir || ir->partitions.empty())
        return;

    const int channelIndex = std::min(channel, ir->numChannels - 1);
    const int bins = fftSize / 2 + 1;

    // Prepare input FFT buffer
    std::fill(tempFreq.begin(), tempFreq.end(), 0.0f);
    std::copy(samples + chunkOffset, samples + chunkOffset + chunkSize, tempFreq.begin());
    fft->performRealOnlyForwardTransform(tempFreq.data());

    auto& channelSpectra = inputSpectra[static_cast<size_t>(channel)];
    auto& writePos = writePositions[static_cast<size_t>(channel)];
    channelSpectra[static_cast<size_t>(writePos)] = tempFreq; // store current block spectrum

    // Accumulate frequency response
    std::fill(accumFreq.begin(), accumFreq.end(), 0.0f);
    for (int p = 0; p < ir->numPartitions; ++p)
    {
        const int idx = (writePos - p);
        const int inputIndex = (idx < 0 ? idx + ir->numPartitions : idx);
        const auto& X = channelSpectra[static_cast<size_t>(inputIndex)];
        const auto& H = ir->partitions[channelIndex][static_cast<size_t>(p)];

        for (int k = 0; k < bins; ++k)
        {
            const int bi = k * 2;
            const float xr = X[bi];
            const float xi = X[bi + 1];
            const float hr = H[bi];
            const float hi = H[bi + 1];

            accumFreq[bi]     += (xr * hr) - (xi * hi);
            accumFreq[bi + 1] += (xr * hi) + (xi * hr);
        }
    }

    // IFFT
    fft->performRealOnlyInverseTransform(accumFreq.data());
    const float scale = 1.0f / static_cast<float>(fftSize);

    auto& overlap = overlapBuffers[static_cast<size_t>(channel)];
    const float dryMix = 1.0f - wetMix;

    for (int n = 0; n < chunkSize; ++n)
    {
        const float wet = (accumFreq[n] * scale) + overlap[n];
        samples[chunkOffset + n] = outputGain * (wetMix * wet + dryMix * dryCopy[chunkOffset + n]);
    }

    // Shift existing overlap forward by chunkSize samples
    if (chunkSize < fftSize)
    {
        const int remain = fftSize - chunkSize;
        std::move(overlap.begin() + chunkSize, overlap.end(), overlap.begin());
        std::fill(overlap.begin() + remain, overlap.end(), 0.0f);

        // Add new tail into overlap
        const int tail = remain;
        for (int i = 0; i < tail; ++i)
            overlap[i] += accumFreq[chunkSize + i] * scale;
    }

    writePos = (writePos + 1) % std::max(1, ir->numPartitions);
}
