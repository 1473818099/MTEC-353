#include <JuceHeader.h>
#include <iostream>

// ================================================================
// Base Audio Effect
// ================================================================

class AudioEffect
{
protected:
    float sampleRate;

public:
    AudioEffect() : sampleRate(44100.0f) {}
    virtual ~AudioEffect() = default;

    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual juce::String getEffectName() const = 0;

    void setSampleRate(double sr) { sampleRate = (float) sr; }
};

// ================================================================
// Tremolo
// ================================================================

class TremoloEffect : public AudioEffect
{
private:
    float lfoFrequency, depth;
    float lfoPhase;

public:
    TremoloEffect(float freq = 5.0f, float dep = 0.5f)
        : lfoFrequency(freq), depth(juce::jlimit(0.0f, 1.0f, dep)), lfoPhase(0.0f) {}

    void process(juce::AudioBuffer<float>& buffer) override
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        float twoPi = juce::MathConstants<float>::twoPi;
        float phaseInc = twoPi * lfoFrequency / sampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            float lfo = std::sin(lfoPhase);
            float unipolar = 0.5f * (lfo + 1.0f);
            float modGain = (1.0f - depth) + depth * unipolar;

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * modGain);

            lfoPhase += phaseInc;
            if (lfoPhase > twoPi) lfoPhase -= twoPi;
        }
    }

    juce::String getEffectName() const override { return "Tremolo"; }
};

// ================================================================
// AD Envelope
// ================================================================

class EnvelopeEffect : public AudioEffect
{
private:
    float attackTime, decayTime;

public:
    EnvelopeEffect(float attack = 0.01f, float decay = 0.1f)
        : attackTime(attack), decayTime(decay) {}

    void process(juce::AudioBuffer<float>& buffer) override
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        int attackSamples = juce::jmax(1, (int)(attackTime * sampleRate));
        int decaySamples  = juce::jmax(1, (int)(decayTime  * sampleRate));
        int total = attackSamples + decaySamples;

        for (int s = 0; s < numSamples; ++s)
        {
            float env = 0.0f;

            if (s < attackSamples)
                env = (float)s / attackSamples;
            else if (s < total)
                env = 1.0f - (float)(s - attackSamples) / decaySamples;
            else
                env = 0.0f;

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * env);
        }
    }

    juce::String getEffectName() const override { return "AD Envelope"; }
};

// ================================================================
// Noise Gate
// ================================================================

class NoiseGateEffect : public AudioEffect
{
private:
    float threshold, attackTime, releaseTime;
    float currentGain;

public:
    NoiseGateEffect(float thresh = 0.02f, float attack = 0.001f, float release = 0.01f)
        : threshold(thresh), attackTime(attack), releaseTime(release), currentGain(0.0f) {}

    void process(juce::AudioBuffer<float>& buffer) override
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        float attackCoeff  = std::exp(-1.0f / (attackTime  * sampleRate));
        float releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));

        for (int s = 0; s < numSamples; ++s)
        {
            float level = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                level = std::max(level, std::abs(buffer.getSample(ch, s)));

            float target = (level > threshold ? 1.0f : 0.0f);
            bool opening = target > currentGain;
            float coeff  = opening ? attackCoeff : releaseCoeff;

            currentGain = coeff * currentGain + (1.0f - coeff) * target;

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * currentGain);
        }
    }

    juce::String getEffectName() const override { return "Noise Gate"; }
};

// ================================================================
// Effect Chain
// ================================================================

class EffectChain
{
private:
    juce::OwnedArray<AudioEffect> effects;

public:
    void addEffect(AudioEffect* fx) { effects.add(fx); }
    void clearChain() { effects.clear(); }

    void setSampleRateForAll(double sr)
    {
        for (auto* fx : effects) fx->setSampleRate(sr);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        for (auto* fx : effects) fx->process(buffer);
    }

    void printChain() const
    {
        std::cout << "Effect Chain:\n";
        for (int i = 0; i < effects.size(); ++i)
            std::cout << "  [" << i << "] " << effects[i]->getEffectName() << "\n";
        std::cout << std::endl;
    }
};

// ================================================================
// WAV Load / Save
// ================================================================

bool loadAudioFile(const juce::File& file,
                   juce::AudioBuffer<float>& buffer,
                   double& sampleRateOut)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    auto reader = std::unique_ptr<juce::AudioFormatReader>(fm.createReaderFor(file));
    if (!reader) return false;

    sampleRateOut = reader->sampleRate;
    buffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&buffer, 0, buffer.getNumSamples(), 0, true, true);
    return true;
}

bool saveAudioFile(const juce::File& file,
                   const juce::AudioBuffer<float>& buffer,
                   double sampleRate)
{
    juce::WavAudioFormat wav;
    auto out = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream());
    if (!out) return false;

    auto writer = std::unique_ptr<juce::AudioFormatWriter>(
        wav.createWriterFor(out.get(), sampleRate,
                            buffer.getNumChannels(), 16, {}, 0));

    if (!writer) return false;
    out.release();
    return writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
}

// ================================================================
// Main
// ================================================================

int main(int argc, char* argv[])
{
    std::cout << "LabExercise4 - DSP Effect Chain\n";

    juce::File inputFile  = (argc > 1 ? juce::File(argv[1])
                                      : juce::File("audio/input.wav"));
    juce::File outputFile = (argc > 2 ? juce::File(argv[2])
                                      : juce::File("audio/output/processed.wav"));

    std::cout << "Input:  " << inputFile.getFullPathName() << "\n";
    std::cout << "Output: " << outputFile.getFullPathName() << "\n";

    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;

    if (!loadAudioFile(inputFile, buffer, sampleRate))
    {
        std::cerr << "Load failed.\n";
        return 1;
    }

    EffectChain chain;
    chain.addEffect(new TremoloEffect(5.0f, 0.6f));
    chain.addEffect(new EnvelopeEffect(0.05f, 0.2f));
    chain.addEffect(new NoiseGateEffect(0.03f, 0.001f, 0.05f));
    chain.setSampleRateForAll(sampleRate);

    chain.printChain();

    chain.process(buffer);

    outputFile.getParentDirectory().createDirectory();
    if (!saveAudioFile(outputFile, buffer, sampleRate))
    {
        std::cerr << "Save failed.\n";
        return 1;
    }

    std::cout << "Done.\n";
    return 0;
}
