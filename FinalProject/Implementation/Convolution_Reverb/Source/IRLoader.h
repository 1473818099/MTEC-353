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

    int computeFFTOrder(int blockSize, int irLength) const;
    std::vector<float> makeMono(const juce::AudioBuffer<float>& buffer);
};
