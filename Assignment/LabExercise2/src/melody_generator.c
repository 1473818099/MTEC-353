#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Part A: Musical Data Structure Design

typedef enum {
    MAJOR = 1,
    MINOR = 2,
    PENTATONIC = 3
} Scale; 

typedef struct Note {
    int   pitch;    
    float duration; 
    int   velocity; 
} Note; 

static const int MAJOR_SCALE[]      = {0, 2, 4, 5, 7, 9, 11};
static const int MINOR_SCALE[]      = {0, 2, 3, 5, 7, 8, 10};
static const int PENTATONIC_SCALE[] = {0, 2, 5, 7, 9};
static const size_t MAJOR_LEN = sizeof(MAJOR_SCALE)/sizeof(MAJOR_SCALE[0]);
static const size_t MINOR_LEN = sizeof(MINOR_SCALE)/sizeof(MINOR_SCALE[0]);
static const size_t PENTA_LEN = sizeof(PENTATONIC_SCALE)/sizeof(PENTATONIC_SCALE[0]);


static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}


static double midi_to_frequency(int midi)
{
    if (midi < 0) midi = 0;
    if (midi > 127) midi = 127;
    return 440.0 * pow(2.0, (midi - 69) / 12.0);
}


static float random_duration(void)
{
    static const float durations[] = {0.25f, 0.5f, 1.0f, 2.0f};
    size_t idx = (size_t)(rand() % 4);
    return durations[idx];
}


static const int* get_scale_array(Scale s, size_t* outLen)
{
    switch (s) {
        case MAJOR:      *outLen = MAJOR_LEN; return MAJOR_SCALE;
        case MINOR:      *outLen = MINOR_LEN; return MINOR_SCALE;
        case PENTATONIC: *outLen = PENTA_LEN; return PENTATONIC_SCALE;
        default:         *outLen = MAJOR_LEN; return MAJOR_SCALE;
    }
}


// Part B: Melody Generation Engine

void generate_melody(Note* melody, int length, Scale scale_type, int root_note)
{
    if (!melody || length <= 0) return;

    size_t scaleLen = 0;
    const int* scale = get_scale_array(scale_type, &scaleLen);
    const int minPitch = clampi(root_note - 24, 0, 127);
    const int maxPitch = clampi(root_note + 24, 0, 127);

    int prevPitch = root_note;

    for (int i = 0; i < length; ++i) {
        Note* n = melody + i;

        int degree = scale[rand() % (int)scaleLen];
        int stepBias = rand() % 100;
        int octaveOffset = 0;
        int pitch = root_note + degree;

        if (i > 0 && stepBias < 70) {
            int deltaDegreeChoices[] = {-1, 0, +1};
            int dd = deltaDegreeChoices[rand() % 3];
            int prevRel = (prevPitch - root_note) % 12;
            if (prevRel < 0) prevRel += 12;
            int nearestIdx = 0;
            int bestDist = 128;
            for (size_t k = 0; k < scaleLen; ++k) {
                int d = abs(scale[k] - prevRel);
                if (d < bestDist) { bestDist = d; nearestIdx = (int)k; }
            }
            int newIdx = nearestIdx + dd;
            if (newIdx < 0) {
                newIdx += (int)scaleLen;
                octaveOffset = -1;
            } else if (newIdx >= (int)scaleLen) {
                newIdx -= (int)scaleLen;
                octaveOffset = +1;
            }
            pitch = root_note + scale[newIdx] + 12 * octaveOffset;
        } else {
            octaveOffset = (rand() % 5) - 2;
            pitch = root_note + degree + 12 * octaveOffset;
        }

        pitch = clampi(pitch, minPitch, maxPitch);
        n->pitch = pitch;
        n->duration = random_duration();
        n->velocity = 64 + (rand() % (127 - 64 + 1));
        prevPitch = pitch;
    }
}


// Part C: Interactive CLI and Output


static void print_scale_menu(void)
{
    printf("Choose scale type:\n");
    printf("  1) MAJOR\n");
    printf("  2) MINOR\n");
    printf("  3) PENTATONIC\n");
}

static void print_ascii_rhythm(float beats)
{
    int dashes = (int)roundf(beats * 4.0f);
    if (dashes < 1) dashes = 1;
    printf("[");
    for (int i = 0; i < dashes; ++i) putchar('-');
    printf("]");
}

int main(void)
{
    srand((unsigned) time(NULL));

    printf("=== Random Melody Generator (C, Pointers, Data Types) ===\n");

    int length = 0;
    do {
        printf("Enter melody length (4-16): ");
        if (scanf("%d", &length) != 1) {
            fprintf(stderr, "Invalid input.\n");
            return 1;
        }
        if (length < 4 || length > 16) {
            printf("Please choose a length between 4 and 16.\n");
        }
    } while (length < 4 || length > 16);

    print_scale_menu();
    int scaleChoice = 0;
    printf("Select (1-3): ");
    if (scanf("%d", &scaleChoice) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }
    Scale scale = (scaleChoice == 2) ? MINOR : (scaleChoice == 3) ? PENTATONIC : MAJOR;

    int root = 60; 
    printf("Enter root MIDI note (e.g., C4=60, D4=62): ");
    if (scanf("%d", &root) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }
    root = clampi(root, 0, 127);

    Note melody[32];
    memset(melody, 0, sizeof(melody));

    generate_melody(melody, length, scale, root);

    const char* scaleName = (scale == MINOR) ? "Minor" : (scale == PENTATONIC) ? "Pentatonic" : "Major";
    printf("\nGenerated Melody in MIDI %d %s (%d notes):\n", root, scaleName, length);

    for (int i = 0; i < length; ++i) {
        Note* n = melody + i; 
        double hz = midi_to_frequency(n->pitch);
        printf("Note %2d: MIDI %3d (%.2f Hz) - Duration: %.2f beats ",
               i+1, n->pitch, hz, n->duration);
        print_ascii_rhythm(n->duration);
        printf("  Vel:%d\n", n->velocity);
    }

    printf("\nre-run to try different notes.\n");
    return 0;
}
