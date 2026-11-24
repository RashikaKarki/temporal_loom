#pragma once

/**
 * @file sparks.h
 * @brief Particle sparks thrown off the loom rim under heavy load.
 *
 * Sparks are emitted when the busiest load exceeds 75 %, more often as load
 * rises, and fall under a light gravity until they fade out.
 */

namespace sparks {

/**
 * Moves existing sparks and possibly emits a new one.
 * @param elapsedFrames Elapsed time in reference frames (1 = 16.667 ms).
 * @param emitting      False clears all sparks immediately (e.g. off the loom screen or unlinked).
 */
void update(float elapsedFrames, bool emitting);
/** Draws the active sparks within the loom area of the current band. */
void draw();

}
