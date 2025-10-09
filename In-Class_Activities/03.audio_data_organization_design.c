float frequency, amplitude, duration;
int sample_rate, bit_depth, channels;
char instrument_name[20], note_name[5];

sample_rate = 44100;
bit_depth = 16;
channels = 2;

typedef struct {
    char instrument_name[20];  
    float frequency;        
    float amplitude;          
    float duration;            
} Instrument;