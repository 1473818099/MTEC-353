/*
  ==============================================================================

    Synth.h
    
    A synthesizer class facilitating basic synthesizer operations and
    state management.
    
    Created: 16 Aug 2021 9:21:07pm
    Author:  Akito van Troyer

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Synth
{
public:
    // Function prototypes for preparing to play and processing audio blocks, to be defined elsewhere.
    void prepareToPlay (int samplesPerBlock, double sampleRate, int numChans);
    void processBlock (juce::AudioBuffer<float>& buffer);

    /**
     * Trigger the ADSR envelope to start (i.e., a note-on event).
     */
    void on()
    {
        adsr.noteOn();
    }

    /**
     * Trigger the ADSR envelope to stop (i.e., a note-off event).
     */
    void off()
    {
        adsr.noteOff();
    }
    
    /**
     * Set the MIDI note number, adjusting the oscillator's frequency accordingly.
     * @param note The MIDI note number.
     */
    void setNote (const int note)
    {
        this->note = note;
        processorChain.get<oscIndex>().setFrequency (juce::MidiMessage::getMidiNoteInHertz (note));
    }

    /**
     * Retrieve the currently set MIDI note number.
     * @return The MIDI note number.
     */
    int getNote () const
    {
        return note;
    }
    
    /**
     * Set the gain linearly based on MIDI velocity (0-127).
     * @param velocity The MIDI note velocity.
     */
    void setVelocity (const int velocity)
    {
        processorChain.get<gainIndex>().setGainLinear (velocity / 127.0f);
    }
    
    /**
     * Check if the ADSR envelope is active (i.e., producing a non-zero output).
     * @return True if the ADSR is active, otherwise false.
     */
    bool isActive () const
    {
        return adsr.isActive();
    }
    
    /**
     * Retrieve the current ADSR parameters.
     * @return A structure containing the current ADSR parameter settings.
     */
    const juce::ADSR::Parameters getADSRParameters() const
    {
        return adsr.getParameters();
    }
    
    /**
     * Set the ADSR parameters.
     * @param parameters A structure containing the desired ADSR parameter settings.
     */
    void setADSRParameters (const juce::ADSR::Parameters parameters)
    {
        adsr.setParameters (parameters);
    }
    
private:
    // Enumeration to facilitate access to the processor chain's components by index.
    enum
    {
        oscIndex,  // Index for the oscillator in the processor chain.
        gainIndex  // Index for the gain processor in the processor chain.
    };
    
    // Temporary buffer for audio data.
    juce::AudioBuffer<float> tempBuffer;

    // The number of channels to process.
    int numChans = 2;

    // The MIDI note number currently set.
    int note = 0;

    // Processor chain containing an oscillator and a gain processor.
    juce::dsp::ProcessorChain<juce::dsp::Oscillator<float>, juce::dsp::Gain<float>> processorChain;

    // ADSR envelope to control the amplitude shape of produced notes.
    juce::ADSR adsr;
};

