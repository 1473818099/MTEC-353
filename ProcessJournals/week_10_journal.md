# Technical Challenge & Solution Process: What specific concepts, algorithms, or implementation problems did you encounter and solve?

- Was discusing how audio data actually moves through the system. From readSoundFile() to writeSoundFile(). It wasn’t obvious whether the buffer stored samples by channel or sequentially per frame. Then we guess each channel has its own array of float samples.

# Collaborative Learning Insights: What did you learn from the activities and your teammates that you wouldn't have discovered working alone?

- My teammate noticed that the printed sample values after the process() doesn't match the input. That's the point I might not able to figure out or will have to spend a lot time to find out a simple mistake.

# Reflection & Next Steps: What questions or concepts do you want to explore further?

I will explore more about clipping, digital clipping.