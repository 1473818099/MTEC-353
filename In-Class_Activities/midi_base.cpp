#include <iostream>
#include <string>

class MIDIMessage
{
protected:
    unsigned char channel;
    unsigned long timestamp; // Simple timing in ms or ticks

public:
    MIDIMessage(unsigned char ch, unsigned long ts = 0) : channel(ch), timestamp(ts) {}
    virtual ~MIDIMessage() = default;

    // Polymorphic send so derived messages can print/send more info
    virtual void send() const
    {
        std::cout << "[MIDI] channel=" << int(channel)
                  << " timestamp=" << timestamp << std::endl;
    }

    unsigned char getChannel() const { return channel; }
    unsigned long getTimestamp() const { return timestamp; }
    void setTimestamp(unsigned long ts) { timestamp = ts; }
};

class MIDINoteMessage : public MIDIMessage
{
private:
    unsigned char note;     // 0-127
    unsigned char velocity; // 0-127
    bool noteOn;            // true = note on, false = note off

public:
    MIDINoteMessage(unsigned char ch, unsigned char n, unsigned char vel, bool on = true, unsigned long ts = 0)
        : MIDIMessage(ch, ts), note(n), velocity(vel), noteOn(on) {}

    // Override send to include note-specific info
    void send() const override
    {
        std::cout << "[MIDI Note] channel=" << int(channel)
                  << " note=" << int(note)
                  << " velocity=" << int(velocity)
                  << " " << (noteOn ? "ON" : "OFF")
                  << " timestamp=" << timestamp << std::endl;
    }

    unsigned char getNote() const { return note; }
    unsigned char getVelocity() const { return velocity; }
    bool isNoteOn() const { return noteOn; }

    void setNoteOn(bool on) { noteOn = on; }
    void setVelocity(unsigned char vel) { velocity = vel; }
};

int main()
{
    // Base message
    MIDIMessage baseMsg(1);
    baseMsg.setTimestamp(100);
    baseMsg.send();

    // Note message (polymorphism)
    MIDINoteMessage noteMsg(2, 60, 100, true, 200); // Middle C on
    MIDIMessage* msgPtr = &noteMsg; // use base pointer
    msgPtr->send();                  // calls MIDINoteMessage::send()

    // Change to note-off and send again
    noteMsg.setNoteOn(false);
    noteMsg.setTimestamp(300);
    noteMsg.send(); // direct call

    return 0;
}