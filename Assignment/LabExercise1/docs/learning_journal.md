# Part E: Learning Journal Entry

## Data Structure Benefits
- Using `struct Note` made the code much easier to read. Without it, you need three arrays for pitch, duration, and velocity. With the struct, all the note information stays together in one place.  
- Using `enum` is better. I can read `MAJOR` or `MINOR` instead of guessing what `1` or `2` means. This makes the code more readable .
- `typedef` helped shorten the code. The trade-off is that it hides the fact that it is a struct.

## Pointer Operations Analysis
- When I compared pointer arithmetic with array indexing, the performance was almost the same, but in my mind, the logic is more direct.
- I learned that arrays are stored in continuous memory. This means pointer arithmetic just moves to the next block in memory.  
```bash
Note* n = melody + i;
n->pitch = 60;
```

## AI as Learning Partner for Systems Concepts
- Text book contains everything about the concepts, while AI will lead me steps by steps and also giving me examples for me to better understand.
- Sometimes AI tried to make a statement/conclusion for me to better understand, but when the information going too much, the statements are making me confused because there're too many long statements.
- I learned to ask short, clear questions. If I ask “why” and “how” about memory, I get better answers than asking very general things.  

## Audio Programming Connections
- Melody generation benefits from efficient memory access because sometimes we need MIDI notes to playback as quickly as possible.
- In real audio programming, DSP must run very fast in real time, so memory organization are important to keep the sound smooth.  
