#pragma once

/**
 * @file boot_sequence.h
 * @brief Typewriter-style start-up text shown before the first screen.
 */

namespace boot_sequence {

/** Starts (or restarts) the boot sequence from the first character. */
void begin();
/** Returns true until all lines have been typed and the final hold time has passed. */
bool isRunning();
/**
 * Advances the typing animation (about 1.3 characters per reference frame).
 * @param elapsedFrames Elapsed time in reference frames (1 = 16.667 ms).
 */
void update(float elapsedFrames);
/** Draws the lines typed so far and the blinking cursor onto the current band. */
void draw();

}
