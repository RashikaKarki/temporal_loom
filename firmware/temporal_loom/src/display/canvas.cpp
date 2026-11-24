/**
 * @file canvas.cpp
 * @brief Frame buffer allocation, banded presentation and text/glitch helpers.
 */
#include <Arduino.h>
#include "canvas.h"
#include "theme.h"

namespace canvas {

static M5Canvas frameBuffer(&M5.Display);
static int currentBandHeight = HEIGHT;
static int currentBandTop = 0;

M5Canvas& surface() { return frameBuffer; }

/** Creates the sprite with the given height in PSRAM or internal RAM and records the band height. */
static bool allocate(int height, bool usePsram) {
  frameBuffer.setPsram(usePsram);
  if (!frameBuffer.createSprite(WIDTH, height)) return false;
  currentBandHeight = height;
  frameBuffer.setFont(&fonts::Font0);
  return true;
}

bool begin(bool preferFullFrame) {
  frameBuffer.deleteSprite();
  frameBuffer.setColorDepth(16);
  if (preferFullFrame && allocate(HEIGHT, true)) return true;
  return allocate(HEIGHT / 2, false);
}

int bandHeight() { return currentBandHeight; }

int toBand(float screenY) { return (int)screenY - currentBandTop; }

void beginBand(int top) {
  currentBandTop = top;
  frameBuffer.fillSprite(theme::background);
  for (int y = 0; y < currentBandHeight; y += 3) frameBuffer.drawFastHLine(0, y, WIDTH, theme::scanline);
}

void presentBand() { frameBuffer.pushSprite(0, currentBandTop); }

void drawText(const char* text, int x, float y, uint16_t color, int size, textdatum_t alignment) {
  frameBuffer.setTextSize(size);
  frameBuffer.setTextColor(color);
  frameBuffer.setTextDatum(alignment);
  frameBuffer.drawString(text, x, toBand(y));
}

void applyGlitch(int slices) {
  for (int slice = 0; slice < slices; slice++) {
    int y = random(0, currentBandHeight - 12);
    int height = random(3, 12);
    int shift = random(-9, 10);
    if (shift > 0) frameBuffer.copyRect(shift, y, WIDTH - shift, height, 0, y);
    else if (shift < 0) frameBuffer.copyRect(0, y, WIDTH + shift, height, -shift, y);
  }
}

}
