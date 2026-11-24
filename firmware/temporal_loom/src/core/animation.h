#pragma once

/**
 * @file animation.h
 * @brief Frame timing and shared animation state.
 *
 * Time is measured in "reference frames" of 1/60 s (16.667 ms) so animations run
 * at the same speed regardless of the actual frame rate. Also provides the alarm
 * shake offsets and the blink timers used by the screens and LED bars.
 */
#include <stdint.h>

namespace animation {

/** Free-running animation clock in reference frames; drives wobbles, pulses and breathing. */
extern float phase;
/** Rotation of the CPU ring in radians; turns clockwise faster as CPU load rises. */
extern float cpuRingAngle;
/** Rotation of the GPU ring in radians; turns counter-clockwise faster as GPU load rises. */
extern float gpuRingAngle;
/** Number of frames rendered since boot; drives the blink and alarm flash cycles. */
extern uint32_t frameCount;
/** Horizontal loom offset in pixels applied while the alarm shake is active (-2 to 2). */
extern int shakeOffsetX;
/** Vertical loom offset in pixels applied while the alarm shake is active (-1 to 1). */
extern int shakeOffsetY;

/**
 * Returns the time since the previous call expressed in reference frames
 * (1 = 16.667 ms), clamped to 4 to avoid large jumps after stalls.
 * @param now Current time in ms (millis()). Call exactly once per loop.
 */
float measureElapsedFrames(uint32_t now);
/**
 * Advances the phase and ring angles by the elapsed time and updates the shake offsets.
 * @param elapsedFrames Elapsed time in reference frames.
 * @param shakeAllowed  True to shake the loom briefly at the start of each alarm cycle.
 */
void advance(float elapsedFrames, bool shakeAllowed);
/** Marks the end of a rendered frame by incrementing frameCount. */
void finishFrame();
/** Returns true during the "on" part of the alarm flash cycle (20 of every 36 frames). */
bool alarmFlashVisible();
/** Returns true during the "on" half of a slow blink that toggles every 30 frames. */
bool slowBlinkVisible();
/** Returns true during the "on" half of the text cursor blink that toggles every 15 frames. */
bool cursorBlinkVisible();

}
