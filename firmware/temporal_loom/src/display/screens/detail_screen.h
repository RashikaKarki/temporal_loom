#pragma once

/**
 * @file detail_screen.h
 * @brief Text screen listing exact values and device status.
 */
#include "../../core/frame_context.h"

namespace detail_screen {

/**
 * Draws rows for CPU and GPU (load and °C), RAM and VRAM (percent and GB),
 * the top process, link state, sound state and, when available, the LED bars.
 * Values use the raw received numbers rather than the smoothed ones.
 */
void draw(const FrameContext& frame);

}
