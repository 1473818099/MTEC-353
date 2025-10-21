/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>

//==============================================================================
int main (int argc, char* argv[])
{
    // Prompt for the file path
    std::cout << "Enter the path to the wave file: ";
    std::string filePath;
    std::getline(std::cin, filePath);
    
    // Create a File object from the path
    juce::File waveFile (filePath);
    
    // Create an audio format manager and register the wav and aiff format
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Create a reader for the wave file
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(waveFile));

    // Print the wave file info if the AudioFormatReader can open the file
    if (reader)
    {
        std::cout << "Sample Rate: " << reader->sampleRate << " Hz" << std::endl;
        std::cout << "Number of Channels: " << reader->numChannels << std::endl;
        std::cout << "Number of Samples: " << reader->lengthInSamples << std::endl;

        double duration = reader->lengthInSamples / reader->sampleRate;
        std::cout << "Duration: " << duration << " seconds" << std::endl;
    }
    else
    {
        std::cout << "Failed to read the wave file." << std::endl;
    }
}