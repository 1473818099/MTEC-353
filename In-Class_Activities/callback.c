// 1st
void callback
void caller(void(*ptr)());

// 2nd
int main() {
    void (*ptr)() = &callback;
    caller(ptr)
}

// 3rd 
void callback() {
    printf("Callback function called.\n");
}

// 4th
void caller(void(*ptr)()) {
    printf("Caller.\n");
    (*ptr)();
}