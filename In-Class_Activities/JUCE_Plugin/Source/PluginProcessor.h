#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class MyPluginAudioProcessor : public juce::AudioProcessor
{
public:
    MyPluginAudioProcessor();
    ~MyPluginAudioProcessor() override = default;

    // Basic info
    const juce::String getName() const override;

    // MIDI / tail
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Programs (we'll just use 1)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // Audio lifecycle
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // 🔊 DSP
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // No GUI for now
    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
};
