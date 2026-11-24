/**
 * @file boot_sequence.cpp
 * @brief Typewriter boot text with a pause between lines and a blinking cursor.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "boot_sequence.h"
#include "../canvas.h"
#include "../theme.h"
#include "../../core/animation.h"

namespace boot_sequence {

static const char* const LINES[] = {"TVA TERMINAL v1.0", "LINKING SACRED TIMELINE...", "CALIBRATING LOOM... OK",
                                     "THREADS DETECTED: 23", "STATUS: STABLE"};
static constexpr int LINE_COUNT = sizeof(LINES) / sizeof(LINES[0]);
static constexpr int PAUSE_BETWEEN_LINES = 4;
static constexpr int HOLD_AFTER_TYPING = 60;
static constexpr float CHARACTERS_PER_FRAME = 1.3f;
static constexpr int LEFT = 6;
static constexpr int FIRST_LINE_Y = 50;
static constexpr int LINE_SPACING = 28;
static constexpr int CHARACTER_WIDTH = 12;

static int totalCharacters = 0;
static float typedCharacters = 0;
static bool running = false;

void begin() {
  totalCharacters = 0;
  for (const char* line : LINES) totalCharacters += strlen(line) + PAUSE_BETWEEN_LINES;
  typedCharacters = 0;
  running = true;
}

bool isRunning() { return running; }

void update(float elapsedFrames) {
  if (!running) return;
  typedCharacters += CHARACTERS_PER_FRAME * elapsedFrames;
  if (typedCharacters > totalCharacters + HOLD_AFTER_TYPING) running = false;
}

static uint16_t lineColor(int index) {
  if (index == 0) return theme::amber;
  if (index == LINE_COUNT - 1) return theme::amberWarm;
  return theme::blend(theme::amberDim, theme::amber, 0.7f);
}

void draw() {
  int consumed = 0;
  int cursorX = LEFT, cursorY = FIRST_LINE_Y;
  for (int index = 0; index < LINE_COUNT; index++) {
    int length = strlen(LINES[index]);
    int visible = constrain((int)typedCharacters - consumed, 0, length);
    if (visible > 0) {
      char partial[32];
      memcpy(partial, LINES[index], visible);
      partial[visible] = '\0';
      int y = FIRST_LINE_Y + index * LINE_SPACING;
      canvas::drawText(partial, LEFT, y, lineColor(index), 2, textdatum_t::top_left);
      cursorX = LEFT + visible * CHARACTER_WIDTH;
      cursorY = y;
    }
    consumed += length + PAUSE_BETWEEN_LINES;
  }
  if (animation::cursorBlinkVisible()) {
    canvas::surface().fillRect(cursorX + 2, canvas::toBand(cursorY), 10, 14, theme::amber);
  }
}

}
