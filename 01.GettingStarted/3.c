
#include <stdio.h>

double calculate_period(int bpm) {
	double ms_per_minute = 60000.0;
	double period = ms_per_minute / bpm;
	return period;
}

double calculate_frequency(int bpm) {
	return (double)bpm / 60.0;
}

int main() {
	int tempo = 128;
	printf("Period: %.2f ms\n", calculate_period(tempo));
	printf("Frequency: %.2f Hz\n", calculate_frequency(tempo));
	return 0;
}
