#include <iostream>
#include <cmath>
using namespace std;

class MidiChord {
private:
    unsigned char notes[4];   // Chord notes (up to 4)
    int noteCount;            // Number of notes in chord

public:
    unsigned char root;       // Root note (MIDI number)
    unsigned char velocity;   // Chord velocity
    unsigned char channel;    // MIDI channel

    // Constructor
    MidiChord(unsigned char rootNote, unsigned char velocity, unsigned char channel)
        : root(rootNote), velocity(velocity), channel(channel), noteCount(0) {
        for (int i = 0; i < 4; i++) notes[i] = 0;
    }

    // Build a Major Triad (root, major 3rd, perfect 5th)
    void buildMajorTriad() {
        notes[0] = root;
        notes[1] = root + 4; // Major third
        notes[2] = root + 7; // Perfect fifth
        noteCount = 3;
    }

    // Play chord (simulation)
    void play() {
        cout << "Playing chord on channel " << (int)channel
             << " with velocity " << (int)velocity << ": ";
        for (int i = 0; i < noteCount; i++) {
            cout << (int)notes[i] << " ("
                 << getFrequency(notes[i]) << " Hz) ";
        }
        cout << endl;
    }

    // Stop chord
    void stop() {
        cout << "Stopping chord on channel " << (int)channel << endl;
    }

    // Convert MIDI pitch to frequency (A440 tuning)
    float getFrequency(unsigned char pitch) {
        return 440.0f * pow(2.0f, (pitch - 69) / 12.0f);
    }
};

int main() {
    int rootInput;

    cout << "Enter a MIDI root note (e.g. 60 = Middle C): ";
    cin >> rootInput;

    // Ensure it's within MIDI range
    if (rootInput < 0 || rootInput > 127) {
        cout << "Invalid root note! Must be between 0 and 127." << endl;
        return 1;
    }

    MidiChord chord((unsigned char)rootInput, 100, 1); // velocity=100, channel=1

    chord.buildMajorTriad();
    chord.play();
    chord.stop();

    return 0;
}
