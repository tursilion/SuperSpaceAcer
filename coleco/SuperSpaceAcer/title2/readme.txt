Animated area is 128x136 (in characters, that's 16x17).
That's 2176 bytes, which will take more than a frame to draw. But probably less than 2.

We can either - live with it. It's not that bad.
Or parse the frames for dirty rectangle and update only the characters that changed. Since the ship only moves on two of the 5 transitions, most of the time it'll be pretty small.
