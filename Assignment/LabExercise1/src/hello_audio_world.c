#include <stdio.h>
#include <math.h>

// Parameters (can be adjusted later)
// double frequency = 1.0;   // Frequency in Hz
// double amplitude = 10.0;  // Amplitude in characters

int main() {
    printf("Welcome to MTEC-353!\n\n");

    int width = 70;   // Number of characters per line
    int height = 20;  // Vertical resolution
    int cycles = 2;   // Number of sine cycles to draw

    for (int x = 0; x < width; x++) {
        double radians = (double)cycles * 2.0 * M_PI * x / width;
        double y = sin(radians);

        // Scale y into vertical position
        int pos = (int)((y + 1.0) * (height - 1) / 2.0);

        for (int i = 0; i < height; i++) {
            if (i == pos) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    return 0;
}
