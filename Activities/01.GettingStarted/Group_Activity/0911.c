// WARNING: Codes in this file will not compile and run as is
// This file is meant to provide you with a starting point for your investigation

// Investigate these variables 

#include <stdio.h>

char c = 'A'; 
int x = 5; 
float y = 3.14; 
double z = 3.14159265359; 
short s = 100; 
long l = 1000000;
int main() {
    // ...existing code...
    printf("Size of char:   %zu bytes\n", sizeof(c));
    printf("Size of int:    %zu bytes\n", sizeof(x));
    printf("Size of float:  %zu bytes\n", sizeof(y));
    printf("Size of double: %zu bytes\n", sizeof(z));
    printf("Size of short:  %zu bytes\n", sizeof(s));
    printf("Size of long:   %zu bytes\n", sizeof(l));

    // You can also check directly on the type:
    printf("Size of long long: %zu bytes\n", sizeof(long long));
    return 0;
}