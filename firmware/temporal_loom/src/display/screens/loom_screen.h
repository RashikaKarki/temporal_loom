#pragma once

/**
 * @file loom_screen.h
 * @brief The main animated loom screen.
 */
#include "../../core/frame_context.h"

namespace loom_screen {

/**
 * Draws the loom: counter-rotating CPU and GPU thread rings whose speed follows
 * load, thread thickness follows RAM/VRAM usage and colour follows heat, plus
 * the meter column and the recent-history strip. While unlinked it draws a
 * broken loom with a "TIMELINE SEVERED" message instead.
 */
void draw(const FrameContext& frame);

}
