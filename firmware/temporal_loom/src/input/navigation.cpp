/**
 * @file navigation.cpp
 * @brief Button-driven screen cycling, alarm muting and LED toggling.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "navigation.h"
#include "../config/settings.h"
#include "../core/alert.h"
#include "../hardware/led_bars.h"

namespace navigation {

static constexpr int SCREEN_COUNT = (int)Screen::Count;
static const char* const SCREEN_NAMES[SCREEN_COUNT] = {"LOOM", "HISTORY", "DETAIL"};
static Screen activeScreen = Screen::Loom;
static uint32_t hintsVisibleUntil = 0;

/** Returns the screen the given number of steps away, wrapping around in both directions. */
static Screen offsetFrom(Screen screen, int steps) {
  return (Screen)(((int)screen + steps + SCREEN_COUNT) % SCREEN_COUNT);
}

Screen current() { return activeScreen; }
Screen next() { return offsetFrom(activeScreen, 1); }
Screen previous() { return offsetFrom(activeScreen, -1); }
const char* nameOf(Screen screen) { return SCREEN_NAMES[(int)screen]; }

void handleButtons(uint32_t now) {
  bool anyPressed = false;
  if (M5.BtnA.wasPressed()) {
    activeScreen = previous();
    anyPressed = true;
  }
  if (M5.BtnC.wasPressed()) {
    activeScreen = next();
    anyPressed = true;
  }
  if (M5.BtnB.wasHold() && led_bars::isAvailable()) {
    led_bars::toggle();
    anyPressed = true;
  }
  if (M5.BtnB.wasClicked()) {
    alert::handleMuteButton();
    anyPressed = true;
  }
  if (anyPressed) hintsVisibleUntil = now + settings::BUTTON_HINT_MS;
}

bool hintsVisible(uint32_t now) { return now < hintsVisibleUntil; }

}
