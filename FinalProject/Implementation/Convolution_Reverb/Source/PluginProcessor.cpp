#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Convolution_ReverbAudioProcessor::Convolution_ReverbAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    engine = std::make_unique<ConvolutionEngine>();
}

Convolution_ReverbAudioProcessor::~Convolution_ReverbAudioProcessor() = default;

//==============================================================================
const juce::String Convolution_ReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Convolution_ReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn  = layouts.getMainInputChannelSet();

    if (mainOut.isDisabled() || mainIn.isDisabled())
        return false;

    if (mainOut != mainIn)
        return false;

    return mainOut == juce::AudioChannelSet::mono()
        || mainOut == juce::AudioChannelSet::stereo();
}

//==============================================================================
void Convolution_ReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lastSampleRate.store(sampleRate);
    lastBlockSize.store(samplesPerBlock);

    engine->prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    dryWetSmoothed.reset(sampleRate, 0.02);
    trimSmoothed.reset(sampleRate, 0.02);

    dryWetSmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue("dryWet"));
    trimSmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue("outputTrim"));
}

void Convolution_ReverbAudioProcessor::releaseResources()
{
    engine->reset();
}

//==============================================================================
void Convolution_ReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals guard;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateSmoothers();

    engine->setMix(dryWetSmoothed.getNextValue());
    engine->setOutputTrim(trimSmoothed.getNextValue());
    engine->process(buffer);
}

//==============================================================================
bool Convolution_ReverbAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Convolution_ReverbAudioProcessor::createEditor()
{
    return new Convolution_ReverbAudioProcessorEditor(*this);
}

//==============================================================================
void Convolution_ReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void Convolution_ReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Convolution_ReverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dryWet", "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputTrim", "Output Trim (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    return { params.begin(), params.end() };
}

void Convolution_ReverbAudioProcessor::updateSmoothers()
{
    dryWetSmoothed.setTargetValue(*parameters.getRawParameterValue("dryWet"));
    trimSmoothed.setTargetValue(*parameters.getRawParameterValue("outputTrim"));
}

//==============================================================================
void Convolution_ReverbAudioProcessor::promptForImpulse()
{
    juce::FileChooser chooser("Select Impulse Response", juce::File{}, "*.wav;*.aiff");
    chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser& fc) {
                            auto result = fc.getResult();
                            if (result.existsAsFile())
                                loadImpulse(result);
                        });
}

void Convolution_ReverbAudioProcessor::loadImpulse(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    isLoading.store(true);
    currentIRName = "Loading...";

    loaderFuture = std::async(std::launch::async, [this, file]() {
        auto ir = irLoader.loadIR(file, lastSampleRate.load(), lastBlockSize.load());
        if (ir)
        {
            engine->setIR(ir);
            currentIRName = file.getFileName();
        }
        else
        {
            currentIRName = "Load failed";
        }
        isLoading.store(false);
    });
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Convolution_ReverbAudioProcessor();
}
