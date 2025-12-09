#pragma once

#include <memory>
#include <juce_audio_formats/juce_audio_formats.h>
#include "ConvolutionEngine.h"

class IRLoader
{
public:
    IRLoader();

    std::shared_ptr<IRData> loadIR(const juce::File& file,
                                   double sampleRate,
                                   int blockSize);

private:
    juce::AudioFormatManager formatManager;

    int computePartitionSize(int blockSize) const;
    int computeFFTOrder(int fftSize) const;
    std::vector<float> makeMono(const juce::AudioBuffer<float>& buffer);
};
