/**
 * @file theme.cpp
 * @brief Palette definitions and RGB565 colour helpers.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "theme.h"
#include "../config/settings.h"

namespace theme {

uint16_t background;
uint16_t scanline;
uint16_t amber;
uint16_t amberDim;
uint16_t amberWarm;
uint16_t amberPale;
uint16_t ramAmber;
uint16_t alarmRed;
uint16_t emptySegment;
uint16_t offlineGrey;
uint16_t offlineGreyDark;

void init() {
  background = lgfx::color565(0x14, 0x0D, 0x04);
  scanline = lgfx::color565(0x0C, 0x08, 0x02);
  amber = lgfx::color565(0xFF, 0x9E, 0x1B);
  amberDim = lgfx::color565(0x5A, 0x3A, 0x10);
  amberWarm = lgfx::color565(0xFF, 0xC0, 0x4D);
  amberPale = lgfx::color565(0xFF, 0xE2, 0xB8);
  ramAmber = lgfx::color565(0xB0, 0x6C, 0x16);
  alarmRed = lgfx::color565(0xE8, 0x43, 0x2E);
  emptySegment = lgfx::color565(0x2E, 0x1E, 0x08);
  offlineGrey = lgfx::color565(0x6B, 0x6B, 0x66);
  offlineGreyDark = lgfx::color565(0x4A, 0x4A, 0x46);
}

uint16_t blend(uint16_t from, uint16_t to, float amount) {
  amount = constrain(amount, 0.0f, 1.0f);
  int redFrom = (from >> 11) & 31, greenFrom = (from >> 5) & 63, blueFrom = from & 31;
  int redTo = (to >> 11) & 31, greenTo = (to >> 5) & 63, blueTo = to & 31;
  int red = redFrom + (redTo - redFrom) * amount;
  int green = greenFrom + (greenTo - greenFrom) * amount;
  int blue = blueFrom + (blueTo - blueFrom) * amount;
  return (uint16_t)((red << 11) | (green << 5) | blue);
}

uint16_t heatColor(float level) {
  if (level > settings::CRITICAL_LEVEL) return alarmRed;
  if (level > settings::WARM_LEVEL) return amberWarm;
  return amber;
}

}
