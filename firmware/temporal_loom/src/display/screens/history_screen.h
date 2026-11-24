#pragma once

/**
 * @file history_screen.h
 * @brief Line graph of recent CPU, GPU and RAM usage.
 */
#include "../../core/frame_context.h"

namespace history_screen {

/**
 * Draws the 0-100 % grid and the CPU (bold), GPU (dashed, if present) and RAM
 * series from the usage history, newest on the right, plus a legend when the
 * button hints are hidden. Lines turn grey while unlinked.
 */
void draw(const FrameContext& frame);

}
