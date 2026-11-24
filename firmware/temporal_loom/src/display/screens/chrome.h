#pragma once

/**
 * @file chrome.h
 * @brief Header bar and footer shared by all screens.
 */
#include "../../core/frame_context.h"

namespace chrome {

/**
 * Draws the top bar with the title, the PC clock (colon blinking every 500 ms)
 * and the link/alarm status. The bar is amber normally, red during an alarm and
 * grey while unlinked.
 */
void drawHeader(const FrameContext& frame);
/**
 * Draws the footer. In priority order: the flashing "LOOM AT CAPACITY" alarm
 * banner, the button hints after a press, or the top process on the loom screen.
 */
void drawFooter(const FrameContext& frame);

}
