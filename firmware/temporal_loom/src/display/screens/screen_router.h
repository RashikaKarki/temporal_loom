#pragma once

/**
 * @file screen_router.h
 * @brief Composes a full frame from the header, the active screen and the footer.
 */
#include "../../core/frame_context.h"

namespace screen_router {

/**
 * Draws the header, the screen selected by navigation and the footer onto the
 * current band, adding random glitch slices while the alarm is active.
 */
void draw(const FrameContext& frame);

}
