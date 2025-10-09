// CC, channel, channel voice
// Status Byte
#include <iostream>
using namespace std;

// Abstract Base Class
class MIDIMessage {
protected:
    float timestamp;  // When the message arrived (seconds or samples)
    int channel;      // MIDI channel (1–16)

public:
    // Constructor
    MIDIMessage(float time = 0.0f, int ch = 1) {
        setTimestamp(time);
        setChannel(ch);
    }

    // Virtual destructor (important for base classes)
    virtual ~MIDIMessage() {}

    // Setters
    void setTimestamp(float time) {
        timestamp = time;
    }

    void setChannel(int ch) {
        if (ch < 1 || ch > 16) {
            cerr << "Error: MIDI channel must be between 1 and 16." << endl;
            channel = 1; // Default to channel 1 if invalid
        } else {
            channel = ch;
        }
    }

    // Getter
    int getChannel() const {
        return channel;
    }

    // Pure virtual function – must be implemented by derived classes
    virtual void display() const = 0;
};

// Small subclass: Note On
class NoteOnMessage : public MIDIMessage {
public:
    NoteOnMessage(uint8_t channel, uint8_t note, uint8_t velocity)
        : MIDIMessage(0x90 | (channel & 0x0F), channel, {note, velocity}) {}

    std::string name() const override { return "Note On"; }
};

// Small subclass: Control Change (for demo)
class ControlChangeMessage : public MIDIMessage {
public:
    ControlChangeMessage(uint8_t channel, uint8_t controller, uint8_t value)
        : MIDIMessage(0xB0 | (channel & 0x0F), channel, {controller, value}) {}

    std::string name() const override { return "Control Change"; }
};

int main() {
    NoteOnMessage noteOn(0, 60, 100); // channel 0, middle C, velocity 100
    ControlChangeMessage cc(0, 64, 127); // sustain pedal on

    // Polymorphic printing
    MIDIMessage* messages[] = { &noteOn, &cc };
    for (auto m : messages) {
        m->print(std::cout);
    }

    return 0;
}
