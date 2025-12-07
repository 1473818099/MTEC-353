#pragma once

#include <future>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ConvolutionEngine.h"
#include "IRLoader.h"

class Convolution_ReverbAudioProcessor : public juce::AudioProcessor
{
public:
    Convolution_ReverbAudioProcessor();
    ~Convolution_ReverbAudioProcessor() override;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // UI helpers
    void promptForImpulse();
    void loadImpulse(const juce::File& file);
    juce::String getCurrentIRName() const { return currentIRName; }
    bool isLoadingIR() const { return isLoading.load(); }

    juce::AudioProcessorValueTreeState& getState() { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;
    std::unique_ptr<ConvolutionEngine> engine;
    IRLoader irLoader;

    std::future<void> loaderFuture;
    std::atomic<bool> isLoading{ false };
    juce::String currentIRName{ "None" };

    std::atomic<double> lastSampleRate{ 44100.0 };
    std::atomic<int> lastBlockSize{ 512 };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dryWetSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> trimSmoothed;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateSmoothers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Convolution_ReverbAudioProcessor)
};
