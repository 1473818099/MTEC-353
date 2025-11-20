#include "PluginProcessor.h"

//==============================================================================

MyPluginAudioProcessor::MyPluginAudioProcessor()
    : juce::AudioProcessor (juce::AudioProcessor::BusesProperties()
                              .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                              .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

// ===================== Basic info =====================

const juce::String MyPluginAudioProcessor::getName() const
{
    return "MyPlugin";
}

bool MyPluginAudioProcessor::acceptsMidi() const      { return false; }
bool MyPluginAudioProcessor::producesMidi() const     { return false; }
bool MyPluginAudioProcessor::isMidiEffect() const     { return false; }
double MyPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

// ===================== Programs =====================

int MyPluginAudioProcessor::getNumPrograms()                    { return 1; }
int MyPluginAudioProcessor::getCurrentProgram()                 { return 0; }
void MyPluginAudioProcessor::setCurrentProgram (int)            {}
const juce::String MyPluginAudioProcessor::getProgramName (int) { return {}; }
void MyPluginAudioProcessor::changeProgramName (int, const juce::String&) {}

// ===================== Lifecycle =====================

void MyPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void MyPluginAudioProcessor::releaseResources()
{
}

bool MyPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // only allow mono or stereo
    auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

    // if we have an input, it must match the output
    if (layouts.getMainInputChannelSet() != mainOut)
        return false;

    return true;
}

// ===================== 🔊 DSP lives here =====================

void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);

    const float gain = 0.5f; // example: -6 dB

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* samples = buffer.getWritePointer (channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            samples[i] *= gain; // 👉 your DSP starts here
    }
}

// ===================== State (empty for now) =====================

void MyPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void MyPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

// ===================== JUCE plugin factory =====================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyPluginAudioProcessor();
}
