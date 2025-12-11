#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class Convolution_ReverbAudioProcessorEditor : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit Convolution_ReverbAudioProcessorEditor(Convolution_ReverbAudioProcessor&);
    ~Convolution_ReverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    Convolution_ReverbAudioProcessor& processor;

    juce::TextButton loadButton{ "Load IR" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Label statusLabel;

    juce::Slider dryWetSlider;
    juce::Slider trimSlider;
    juce::Label dryWetLabel;
    juce::Label trimLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> dryWetAttachment;
    std::unique_ptr<SliderAttachment> trimAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Convolution_ReverbAudioProcessorEditor)
};
