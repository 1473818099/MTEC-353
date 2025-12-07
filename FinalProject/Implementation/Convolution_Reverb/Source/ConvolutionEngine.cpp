#include "ConvolutionEngine.h"

ConvolutionEngine::ConvolutionEngine() = default;

void ConvolutionEngine::prepare(double newSampleRate, int newBlockSize, int numChannels)
{
    sampleRate = newSampleRate;
    blockSize = newBlockSize;
    ensureFFTOrder(11); // default start, may be changed by IR
    resizeBuffers(numChannels);
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

    ensureFFTOrder(ir->fftOrder);
    currentIR.store(ir, std::memory_order_release);
    resizeBuffers(ir->numChannels);
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

    resizeBuffers(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
        processChannel(ch, buffer.getWritePointer(ch), numSamples);
}

void ConvolutionEngine::ensureFFTOrder(int desiredOrder)
{
    if (desiredOrder == fftOrder && fft)
        return;

    fftOrder = desiredOrder;
    fftSize = 1 << fftOrder;
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);
    temp.resize(static_cast<size_t>(fftSize * 2), 0.0f);

    for (auto& ch : overlapBuffers)
        ch.resize(static_cast<size_t>(fftSize), 0.0f);
}

void ConvolutionEngine::resizeBuffers(int numChannels)
{
    if (numChannels <= 0)
        return;

    overlapBuffers.resize(static_cast<size_t>(numChannels));
    for (auto& ch : overlapBuffers)
        ch.resize(static_cast<size_t>(fftSize), 0.0f);
}

void ConvolutionEngine::processChannel(int channel, float* samples, int numSamples)
{
    auto ir = currentIR.load(std::memory_order_acquire);
    if (!ir || ir->frequencyData.empty())
        return;

    // Copy input to temp and zero-pad
    std::fill(temp.begin(), temp.end(), 0.0f);
    const int copyCount = std::min(numSamples, fftSize);
    std::copy(samples, samples + copyCount, temp.begin());

    // Forward FFT (real-only)
    fft->performRealOnlyForwardTransform(temp.data());

    const auto& spectrum = ir->frequencyData[std::min(channel, ir->numChannels - 1)];

    // Complex multiply in-place: temp *= spectrum
    const int bins = fftSize / 2 + 1;
    for (int k = 0; k < bins; ++k)
    {
        const int idx = k * 2;
        const float aReal = temp[idx];
        const float aImag = temp[idx + 1];

        const float bReal = spectrum[idx];
        const float bImag = spectrum[idx + 1];

        temp[idx]     = (aReal * bReal) - (aImag * bImag);
        temp[idx + 1] = (aReal * bImag) + (aImag * bReal);
    }

    // Inverse FFT
    fft->performRealOnlyInverseTransform(temp.data());

    auto& overlap = overlapBuffers[static_cast<size_t>(channel)];

    // Overlap-add and mix
    const float dryMix = 1.0f - wetMix;
    for (int n = 0; n < numSamples; ++n)
    {
        const float wet = temp[n] + overlap[n];
        samples[n] = outputGain * (wetMix * wet + dryMix * samples[n]);
    }

    // Update overlap buffer with tail
    const int tailSamples = fftSize - numSamples;
    if (tailSamples > 0)
    {
        std::copy(temp.begin() + numSamples, temp.begin() + numSamples + tailSamples, overlap.begin());
        std::fill(overlap.begin() + tailSamples, overlap.end(), 0.0f);
    }
}
