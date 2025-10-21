/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>

int main()
{
    // Prompt for the input file path
    std::cout << "Enter the path to the source wave file: ";
    std::string sourceFilePath;
    std::getline(std::cin, sourceFilePath);

    // Prompt for the output file path
    std::cout << "Enter the path for the destination wave file: ";
    std::string destFilePath;
    std::getline(std::cin, destFilePath);

    // Create a File object from the path
    juce::File sourceFile(sourceFilePath);
    juce::File destFile(destFilePath);

    // Create an audio format manager and register the wav format
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Create a unique_ptr for the reader
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(sourceFile));

    if (reader)
    {
        // Create a buffer to hold the samples
        juce::AudioBuffer<float> buffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

        // Read the samples into the buffer
        reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

        // Create a unique_ptr for the writer
        std::unique_ptr<juce::AudioFormatWriter> writer(formatManager.findFormatForFileExtension("wav")
                                              ->createWriterFor(new juce::FileOutputStream(destFile),
                                                                 reader->sampleRate,
                                                                 reader->numChannels,
                                                                 16,
                                                                 {},
                                                                 0));
        
        if (writer)
        {
            // Write the samples from the buffer to the destination file
            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
        else
        {
            std::cout << "Failed to create a writer for the destination file." << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to read the source wave file." << std::endl;
    }
}
