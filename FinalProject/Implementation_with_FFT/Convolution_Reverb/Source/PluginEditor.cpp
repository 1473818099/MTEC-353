#include "PluginEditor.h"
#include "PluginProcessor.h"

Convolution_ReverbAudioProcessorEditor::Convolution_ReverbAudioProcessorEditor(Convolution_ReverbAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(420, 240);

    loadButton.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select Impulse Response", juce::File{}, "*.wav;*.aiff");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync(flags,
                                 [this](const juce::FileChooser& fc)
                                 {
                                     auto result = fc.getResult();
                                     if (result.existsAsFile())
                                         processor.loadImpulse(result);
                                 });
    };
    addAndMakeVisible(loadButton);

    statusLabel.setText("IR: None", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);

    dryWetSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    dryWetSlider.setName("Dry/Wet");
    addAndMakeVisible(dryWetSlider);

    trimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    trimSlider.setName("Trim");
    addAndMakeVisible(trimSlider);

    dryWetAttachment = std::make_unique<SliderAttachment>(processor.getState(), "dryWet", dryWetSlider);
    trimAttachment = std::make_unique<SliderAttachment>(processor.getState(), "outputTrim", trimSlider);

    startTimerHz(10);
}

Convolution_ReverbAudioProcessorEditor::~Convolution_ReverbAudioProcessorEditor()
{
    stopTimer();
}

void Convolution_ReverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Convolution Reverb", getLocalBounds().reduced(8, 8).removeFromTop(30),
                     juce::Justification::centredLeft, 1);
}

void Convolution_ReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto header = area.removeFromTop(40);

    loadButton.setBounds(header.removeFromRight(120));
    statusLabel.setBounds(header);

    auto knobs = area.removeFromTop(160);
    auto knobWidth = knobs.getWidth() / 2;
    dryWetSlider.setBounds(knobs.removeFromLeft(knobWidth).reduced(10));
    trimSlider.setBounds(knobs.reduced(10));
}

void Convolution_ReverbAudioProcessorEditor::timerCallback()
{
    juce::String status = "IR: " + processor.getCurrentIRName();
    if (processor.isLoadingIR())
        status += " (loading...)";
    statusLabel.setText(status, juce::dontSendNotification);
}
