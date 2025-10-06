#include <iostream>
#include <cmath>

typedef struct MidiChord
{
    unsigned char root;     // Root note (MIDI number)
    unsigned char notes[4]; // Chord notes
    unsigned char velocity; // Chord velocity
    unsigned char channel;  // MIDI channel
    int noteCount;          // Number of notes in chord
} MidiChord;

void buildMajorTriad(MidiChord *chord);
void playChord(MidiChord *chord);
void stopChord(MidiChord *chord);
float getFrequency(unsigned char pitch);

int main()
{
    MidiChord chord = {60, {0, 0, 0, 0}, 100, 1, 0}; // C major

    buildMajorTriad(&chord);
    playChord(&chord);
    stopChord(&chord);

    return 0;
}




