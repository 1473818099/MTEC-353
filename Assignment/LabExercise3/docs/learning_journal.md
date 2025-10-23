# Learning Journal — Lab Exercise 3

## Framework Overwhelm
Managing CMake paths and setting up JUCE modules felt more complicated than expected for a small console app
I kept testing simple builds, searched error messages, and used AI explanations until everything compiled.
A beginner-friendly visual guide or preconfigured JUCE template would have helped a lot.

## Abstraction Mechanism Insights
It clicked when all my different MIDI events could be processed through one base class pointer.
I’d use templates when I need type flexibility and speed without runtime cost.
Python feels simpler but slower; C++ gives tighter control and faster performance.
I expected virtual functions to be heavy, but they barely affected performance.

## Professional Development Reality Check
It was more complex than I thought, especially with build systems and linking.
Large frameworks take longer to learn but save time later with built-in tools.
AI explained CMake commands, fixed path errors, and clarified JUCE’s structure.

## MIDI System Design Reflection
It helped me see notes and controls as independent actions with their own logic.