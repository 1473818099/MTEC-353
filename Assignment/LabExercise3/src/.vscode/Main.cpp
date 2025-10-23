#include <juce_core/juce_core.h>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>

// ============================================================================
// Part A: Verify JUCE works
// ============================================================================
static void verifyJUCE()
{
    juce::String greeting("JUCE MIDI Event System");
    std::cout << greeting << std::endl;
    std::cout << "Build successful!" << std::endl;
}

// ============================================================================
// Helpers
// ============================================================================
static juce::String noteNumberToName(int noteNumber)
{
    static const char* names[12] = { "C", "C#", "D", "D#", "E", "F",
                                     "F#", "G", "G#", "A", "A#", "B" };
    const int pitchClass = ((noteNumber % 12) + 12) % 12;
    const int octave     = (noteNumber / 12) - 1; // MIDI: C4 = 60 -> octave = 4
    return juce::String(names[pitchClass]) + juce::String(octave);
}

// Template function: MIDI note -> frequency (A4=440, equal temperament)
template <typename T>
T midiNoteToHz(T note)
{
    return static_cast<T>(440.0) * std::pow(static_cast<T>(2.0), (note - static_cast<T>(69)) / static_cast<T>(12.0));
}

// (Example specialization: integer input returns double)
template <>
double midiNoteToHz<int>(int note)
{
    return 440.0 * std::pow(2.0, (note - 69) / 12.0);
}

// ============================================================================
// Part B: Polymorphic MIDI Event Hierarchy
// ============================================================================
class MidiEvent
{
protected:
    int timestamp; // ms
    int channel;   // 1-16

public:
    MidiEvent(int time, int ch) : timestamp(time), channel(ch) {}
    virtual ~MidiEvent() = default;

    int getTime() const     { return timestamp; }
    int getChannel() const  { return channel; }

    // Pure virtual interface
    virtual juce::String toString() const = 0;
    virtual int getDataSize() const = 0;
    virtual void execute() const = 0;
};

class NoteOnEvent : public MidiEvent
{
private:
    int note;     // 0-127
    int velocity; // 0-127

public:
    NoteOnEvent(int time, int ch, int n, int vel)
    : MidiEvent(time, ch), note(n), velocity(vel) {}

    juce::String toString() const override
    {
        return juce::String::formatted("NoteOn: %s vel:%d ch:%d",
            noteNumberToName(note).toRawUTF8(), velocity, channel);
    }

    int getDataSize() const override { return 3; } // status, data1, data2

    void execute() const override
    {
        std::cout << "[Play] note " << note
                  << " (" << noteNumberToName(note) << ", "
                  << midiNoteToHz<double>(note) << " Hz)"
                  << " velocity " << velocity
                  << " on channel " << channel << "\n";
    }
};

class NoteOffEvent : public MidiEvent
{
private:
    int note;
    int velocity; // release velocity (often 0)

public:
    NoteOffEvent(int time, int ch, int n, int vel = 0)
    : MidiEvent(time, ch), note(n), velocity(vel) {}

    juce::String toString() const override
    {
        return juce::String::formatted("NoteOff: %s vel:%d ch:%d",
            noteNumberToName(note).toRawUTF8(), velocity, channel);
    }

    int getDataSize() const override { return 3; }

    void execute() const override
    {
        std::cout << "[Stop] note " << note
                  << " (" << noteNumberToName(note) << ")"
                  << " velocity " << velocity
                  << " on channel " << channel << "\n";
    }
};

class ControlChangeEvent : public MidiEvent
{
private:
    int controller; // CC number 0-127
    int value;      // 0-127

public:
    ControlChangeEvent(int time, int ch, int cc, int val)
    : MidiEvent(time, ch), controller(cc), value(val) {}

    juce::String toString() const override
    {
        return juce::String::formatted("ControlChange: cc:%d val:%d ch:%d",
            controller, value, channel);
    }

    int getDataSize() const override { return 3; }

    void execute() const override
    {
        std::cout << "[CC] controller " << controller
                  << " = " << value
                  << " on channel " << channel << "\n";
    }
};

class ProgramChangeEvent : public MidiEvent
{
private:
    int program; // 0-127

public:
    ProgramChangeEvent(int time, int ch, int pgm)
    : MidiEvent(time, ch), program(pgm) {}

    juce::String toString() const override
    {
        return juce::String::formatted("ProgramChange: program:%d ch:%d",
            program, channel);
    }

    int getDataSize() const override { return 2; } // status, data1

    void execute() const override
    {
        std::cout << "[Program] set program " << program
                  << " on channel " << channel << "\n";
    }
};

// ============================================================================
// Part C: Template-Based Event Container
// ============================================================================
template <typename T>
class TimedEventQueue
{
private:
    struct TimedItem
    {
        int timestamp;
        T   data;
    };

    juce::Array<TimedItem> events;

public:
    void addEvent(int time, T event)
    {
        events.add({ time, std::move(event) });
    }

    // Returns the next event by time (removes it)
    T getNextEvent()
    {
        jassert(events.size() > 0);
        sortByTime();
        auto item = events.removeAndReturn(0);
        return std::move(item.data);
    }

    void sortByTime()
    {
        std::sort(events.begin(), events.end(),
            [](const TimedItem& a, const TimedItem& b)
            {
                return a.timestamp < b.timestamp;
            });
    }

    template <typename Predicate>
    juce::Array<T> filterEvents(Predicate pred)
    {
        juce::Array<T> result;
        for (const auto& e : events)
            if (pred(e.data))
                result.add(e.data);
        return result;
    }

    int size() const { return events.size(); }

    // For demo/printing
    template <typename Printer>
    void forEachOrdered(Printer printer)
    {
        sortByTime();
        for (const auto& e : events)
            printer(e.timestamp, e.data);
    }
};

// ============================================================================
// Demo / Expected Output
// ============================================================================
int main()
{
    std::cout << "=== MIDI Event System ===\n";
    verifyJUCE();

    // Part B: Polymorphism demo
    std::vector<std::shared_ptr<MidiEvent>> seq;
    seq.emplace_back(std::make_shared<NoteOnEvent>(0,   1, 60, 64));   // C4
    seq.emplace_back(std::make_shared<NoteOnEvent>(100, 1, 64, 80));   // E4
    seq.emplace_back(std::make_shared<ControlChangeEvent>(200, 1, 7, 100)); // CC7 volume
    seq.emplace_back(std::make_shared<NoteOffEvent>(300, 1, 60, 0));
    seq.emplace_back(std::make_shared<ProgramChangeEvent>(400, 1, 10));

    std::cout << "\nCreated " << seq.size() << " polymorphic events\n";
    std::cout << "\nProcessing event sequence:\n";
    for (const auto& e : seq)
    {
        std::cout << "[" << e->getTime() << "ms] "
                  << e->toString() << "\n";
        e->execute();
    }

    // Part C: Template queues
    TimedEventQueue<std::shared_ptr<MidiEvent>> midiQueue;
    TimedEventQueue<juce::String>               messageQueue;
    TimedEventQueue<float>                      automationQueue;

    midiQueue.addEvent(200, std::make_shared<ControlChangeEvent>(200, 1, 1, 64));
    midiQueue.addEvent(0,   std::make_shared<NoteOnEvent>(0, 1, 67, 90)); // G4
    midiQueue.addEvent(100, std::make_shared<NoteOffEvent>(100, 1, 67, 0));

    messageQueue.addEvent(150, "hello");
    messageQueue.addEvent(50,  "world");
    messageQueue.addEvent(400, "JUCE");

    automationQueue.addEvent(300, 0.25f);
    automationQueue.addEvent(50,  0.75f);
    automationQueue.addEvent(175, 0.5f);

    std::cout << "\nTemplate queue test: Sorted 3 queues of different types\n";
    std::cout << "midiQueue size=" << midiQueue.size()
              << ", messageQueue size=" << messageQueue.size()
              << ", automationQueue size=" << automationQueue.size() << "\n";

    std::cout << "\nOrdered midiQueue:\n";
    midiQueue.forEachOrdered([](int t, const std::shared_ptr<MidiEvent>& evt)
    {
        std::cout << "[" << t << "ms] " << evt->toString() << "\n";
    });

    std::cout << "\nOrdered messageQueue:\n";
    messageQueue.forEachOrdered([](int t, const juce::String& s)
    {
        std::cout << "[" << t << "ms] " << s << "\n";
    });

    std::cout << "\nOrdered automationQueue:\n";
    automationQueue.forEachOrdered([](int t, float v)
    {
        std::cout << "[" << t << "ms] value=" << v << "\n";
    });

    return 0;
}
