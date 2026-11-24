#pragma once

/**
 * @file frame_context.h
 * @brief Per-frame values passed to every screen while drawing.
 */
#include <stdint.h>

/** Values captured once at the start of each loop and shared by all drawing code. */
struct FrameContext {
  /** Time of this frame in ms (millis()). */
  uint32_t now;
  /** Whether the PC link is currently up. */
  bool linked;
};
