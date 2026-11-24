/**
 * @file pc_link.cpp
 * @brief Line-buffered serial reader for the PC stats stream.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "pc_link.h"
#include "stats_store.h"
#include "../config/settings.h"

namespace pc_link {

static constexpr size_t MAX_LINE_LENGTH = 384;
static char lineBuffer[MAX_LINE_LENGTH];
static size_t lineLength = 0;
static uint32_t lastMessageAt = 0;
static bool hasEverReceived = false;

void begin() {
  Serial.setRxBufferSize(1024);
  Serial.begin(settings::SERIAL_BAUD);
}

/** Terminates the buffered line, applies it as JSON and refreshes the link timestamp on success. */
static void finishLine(uint32_t now) {
  lineBuffer[lineLength] = '\0';
  if (stats::applyJson(lineBuffer)) {
    lastMessageAt = now;
    hasEverReceived = true;
  }
  lineLength = 0;
}

/** Appends a character to the line buffer, discarding the partial line if it overflows. */
static void appendCharacter(char character) {
  const bool lineOverflowed = lineLength >= MAX_LINE_LENGTH - 1;
  if (lineOverflowed) {
    lineLength = 0;
    return;
  }
  lineBuffer[lineLength++] = character;
}

void poll(uint32_t now) {
  while (Serial.available()) {
    char character = (char)Serial.read();
    if (character == '\n') finishLine(now);
    else if (character != '\r') appendCharacter(character);
  }
}

bool isConnected(uint32_t now) { return hasEverReceived && now - lastMessageAt < settings::LINK_TIMEOUT_MS; }

}
