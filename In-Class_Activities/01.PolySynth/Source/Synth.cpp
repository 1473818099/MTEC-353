/*
  ==============================================================================

    Synth.cpp
    Created: 16 Aug 2021 9:21:07pm
    Author:  Akito van Troyer

  ==============================================================================
*/

#include "Synth.h"

void Synth::prepareToPlay (int samplesPerBlock, double sampleRate, int numChans)
{
    // Set up the processing specifications and prepare the processor chain
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;            // Assign the sample rate to the specification
    spec.maximumBlockSize = samplesPerBlock; // Assign the maximum block size to the specification
    spec.numChannels = numChans;             // Assign the number of channels to the specification
    processorChain.prepare (spec);           // Prepare the processor chain with the defined specifications
    
    // Initialize the oscillator and set its frequency
    auto& osc = processorChain.template get<oscIndex>();
    osc.initialise ([] (float x)
    {
        // Map the phase input x to a periodic waveform between -1 and 1
        return juce::jmap (x,
                           float (-juce::MathConstants<float>::pi),
                           float (juce::MathConstants<float>::pi),
                           float (-1),
                           float (1));
    }, 2);
    
    // Set the oscillator frequency to 440Hz
    processorChain.get<oscIndex>().setFrequency (440, true);
        
    // Set the initial gain of the gain processor
    processorChain.get<gainIndex>().setGainLinear (0.05f); // Set the linear gain to 0.05
    
    // Configure the ADSR envelope
    adsr.setSampleRate (sampleRate); // Set the ADSR sample rate
    adsr.reset(); // Reset the state of the ADSR
    
    // Update the class's private member variables
    this->numChans = numChans; // Set the number of channels
    
    // Size the temporary buffer accordingly
    tempBuffer.setSize (numChans, samplesPerBlock); // Set the temporary buffer size
}

void Synth::processBlock (juce::AudioBuffer<float>& buffer)
{
    // Create an audio block linking to the temporary buffer
    juce::dsp::AudioBlock<float> block (tempBuffer);
    
    // Create a processing context for the audio block
    juce::dsp::ProcessContextReplacing<float> context (block);
    
    // Process the audio data through the processor chain
    processorChain.process (context);
    
    // Apply the ADSR envelope to the temporary buffer
    adsr.applyEnvelopeToBuffer (tempBuffer, 0, tempBuffer.getNumSamples());
    
    // Add the processed data from the temporary buffer to the main audio buffer
    for (int channel = 0; channel < numChans; ++channel)
    {
        // Add data per channel from the temporary buffer to the main buffer
        buffer.addFrom (channel,           // Destination channel
                        0,                 // Destination start sample
                        tempBuffer,        // Source
                        channel,           // Source channel
                        0,                 // Source start sample
                        buffer.getNumSamples()); // Number of samples
    }
}
