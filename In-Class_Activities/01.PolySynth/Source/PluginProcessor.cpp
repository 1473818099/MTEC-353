/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SineSynthAudioProcessor::SineSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SineSynthAudioProcessor::~SineSynthAudioProcessor()
{
}

//==============================================================================
const juce::String SineSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SineSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SineSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SineSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SineSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SineSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SineSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SineSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SineSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SineSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SineSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize the synthesizers
    for (int i = 0; i < MAX_SYNTHS; ++i)
    {
        synths.add (new Synth ());
        synths[i]->prepareToPlay(samplesPerBlock, sampleRate, 2);
    }
}


void SineSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SineSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SineSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // ScopedNoDenormals is a helper object that disables denormals
    // for the scope in which it is declared, aiding performance.
    juce::ScopedNoDenormals noDenormals;

    // Process incoming MIDI messages and possibly alter processor state based on them.
    processMidiMessages (midiMessages);
    
    // Clear the audio buffer, ensuring no prior data contaminates the processing.
    buffer.clear();
    
    // Go through each synth in the synths array and pass the audio buffer for processing
    for (Synth* synth : synths)
    {
        synth->processBlock (buffer);
    }
}

/**
 * Processes the incoming MIDI messages to handle note on/off and MIDI control changes.
 * @param midiMessages A buffer containing MIDI messages to be processed.
 */
void SineSynthAudioProcessor::processMidiMessages (juce::MidiBuffer& midiMessages)
{
    // Loop through all MIDI messages in the provided buffer
    for (const juce::MidiMessageMetadata metadata : midiMessages)
    {
        // Extract the MIDI message data from the current metadata object
        juce::MidiMessage message = metadata.getMessage();
 
        // If the message represents a MIDI Note On event
        if (message.isNoteOn())
        {
            // Iterate through all `Synth` objects managed by `synths` OwnedArray
            for (Synth* synth : synths)
            {
                // If the current `synth` is not active (not playing a note)
                if (!synth->isActive())
                {
                    // Set the note number and velocity in the `synth` object
                    // and activate it, then break out of the synth loop
                    synth->setNote (message.getNoteNumber());
                    synth->setVelocity (message.getVelocity());
                    synth->on();
                    break;
                }
            }
        }
        // If the message represents a MIDI Note Off event
        else if (message.isNoteOff())
        {
            // Iterate through all `Synth` objects in `synths` OwnedArray
            for (Synth* synth : synths)
            {
                // If the current `synth` is active and playing the note number
                // in the message, deactivate it and break out of the synth loop
                if (synth->getNote() == message.getNoteNumber() && synth->isActive())
                {
                    synth->off();
                    break;
                }
            }
        }
        // If the message is a Control Change message
        else if (message.isController())
        {
            // Handle the ADSR envelope adjustments via a separate function
            controlADSR (message);
        }
    }
}

/**
 * Uses the incoming MIDI message to handle MIDI control changes for changing the ADSR parameters.
 * @param midiMessage A MIDI message to get the control change value.
 */
void SineSynthAudioProcessor::controlADSR (juce::MidiMessage midiMessage)
{
    // Retrieve the ADSR parameters from the first `Synth` in `synths` array.
    juce::ADSR::Parameters parameters = synths[0]->getADSRParameters();
    
    // Use a switch statement to handle various MIDI controller numbers.
    switch (midiMessage.getControllerNumber()) {
        // In case controller number is 1, map the controller value to the attack parameter of ADSR.
        case 1:
            parameters.attack = juce::jmap(float(midiMessage.getControllerValue()), 0.0f, 127.0f, 0.01f, 0.4f);
            break;
        // In case controller number is 2, map the controller value to the decay parameter of ADSR.
        case 2:
            parameters.decay = juce::jmap(float(midiMessage.getControllerValue()), 0.0f, 127.0f, 0.01f, 0.4f);
            break;
        // In case controller number is 3, map the controller value to the sustain parameter of ADSR.
        case 3:
            parameters.sustain = juce::jmap(float(midiMessage.getControllerValue()), 0.0f, 127.0f, 0.1f, 1.0f);
            break;
        // In case controller number is 4, map the controller value to the release parameter of ADSR.
        case 4:
            parameters.release = juce::jmap(float(midiMessage.getControllerValue()), 0.0f, 127.0f, 0.01f, 0.4f);
            break;
        // Default case does nothing if the controller number doesn't match 1-4.
        default:
            break;
    }
    
    // Loop through all `Synth` objects in `synths` and set their ADSR parameters with the modified `parameters`.
    for (Synth* synth: synths)
    {
        synth->setADSRParameters (parameters);
    }
}

//==============================================================================
bool SineSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SineSynthAudioProcessor::createEditor()
{
    return new SineSynthAudioProcessorEditor (*this);
}

//==============================================================================
void SineSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SineSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SineSynthAudioProcessor();
}
