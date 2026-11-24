#pragma once

/**
 * @file canvas.h
 * @brief Off-screen frame buffer and banded rendering.
 *
 * Draws into an M5Canvas sprite and pushes it to the LCD. A full 320x240 frame
 * buffer in PSRAM is preferred; when that cannot be allocated the frame is
 * rendered in two half-height bands from internal RAM. Drawing code always
 * uses screen coordinates and converts the y position with toBand().
 */
#include <M5Unified.h>

namespace canvas {

/** Screen width in pixels. */
constexpr int WIDTH = 320;
/** Screen height in pixels. */
constexpr int HEIGHT = 240;

/** Returns the sprite to draw into for the current band. */
M5Canvas& surface();
/**
 * Allocates the frame buffer (16-bit colour).
 * @param preferFullFrame Try a full-height buffer in PSRAM first, falling back to a half-height band.
 * @return False when not even the half-height buffer could be allocated.
 */
bool begin(bool preferFullFrame = true);
/** Returns the height in pixels of each rendered band (240 or 120). */
int bandHeight();
/** Converts a screen y coordinate in pixels to a y coordinate within the current band. */
int toBand(float screenY);
/** Starts a band at the given screen row: clears it to the background and draws the scanlines. */
void beginBand(int top);
/** Pushes the current band to the LCD. */
void presentBand();
/**
 * Draws a string on the current band.
 * @param x         Screen x in pixels.
 * @param y         Screen y in pixels (converted to band coordinates internally).
 * @param color     RGB565 colour.
 * @param size      Text scale factor for the built-in 6x8 font.
 * @param alignment Anchor point of the text relative to (x, y).
 */
void drawText(const char* text, int x, float y, uint16_t color, int size, textdatum_t alignment);
/** Shifts the given number of random horizontal slices of the current band sideways for a glitch effect. */
void applyGlitch(int slices);

/**
 * Renders one full frame by calling drawFrame once per band, then pushing each band to the LCD.
 * @param drawFrame Callable that draws the whole screen in screen coordinates.
 */
template <typename DrawFrame>
void render(DrawFrame&& drawFrame) {
  for (int top = 0; top < HEIGHT; top += bandHeight()) {
    beginBand(top);
    drawFrame();
    presentBand();
  }
}

}
