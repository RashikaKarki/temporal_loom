/**
 * @file screen_router.cpp
 * @brief Dispatches drawing to the active screen and applies the alarm glitch.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "screen_router.h"
#include "chrome.h"
#include "detail_screen.h"
#include "history_screen.h"
#include "loom_screen.h"
#include "../canvas.h"
#include "../../core/alert.h"
#include "../../input/navigation.h"

namespace screen_router {

static constexpr int GLITCH_CHANCE_PERCENT = 18;
static constexpr int GLITCH_SLICES_PER_BAND = 2;

static void drawActiveScreen(const FrameContext& frame) {
  switch (navigation::current()) {
    case Screen::Loom: loom_screen::draw(frame); break;
    case Screen::History: history_screen::draw(frame); break;
    case Screen::Detail: detail_screen::draw(frame); break;
    default: break;
  }
}

void draw(const FrameContext& frame) {
  chrome::drawHeader(frame);
  drawActiveScreen(frame);
  chrome::drawFooter(frame);
  const bool glitching = alert::state().alarmActive && random(100) < GLITCH_CHANCE_PERCENT;
  if (glitching) canvas::applyGlitch(GLITCH_SLICES_PER_BAND);
}

}
