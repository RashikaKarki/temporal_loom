/**
 * @file led_bars.cpp
 * @brief Drives the M5Stack Fire's 10 SK6812 LEDs as two five-LED meters.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include <utility/led/LED_Strip_Class.hpp>
#include "led_bars.h"
#include "../config/settings.h"
#include "../core/alert.h"
#include "../core/animation.h"
#include "../core/stats_store.h"

namespace led_bars {

struct Rgb {
  uint8_t red, green, blue;
  Rgb scaled(float amount) const { return {(uint8_t)(red * amount), (uint8_t)(green * amount), (uint8_t)(blue * amount)}; }
};

static constexpr int LEDS_PER_SIDE = 5;
static constexpr int TOTAL_LEDS = LEDS_PER_SIDE * 2;
static constexpr uint32_t REFRESH_INTERVAL_MS = 40;
static constexpr float UNLIT_GLOW = 0.08f;
static constexpr Rgb LED_AMBER = {255, 70, 0};
static constexpr Rgb LED_WARM = {255, 120, 0};
static constexpr Rgb LED_RED = {255, 0, 0};
static constexpr Rgb LED_OFF = {0, 0, 0};

static bool available = false;
static bool switchedOn = settings::LED_BARS_ENABLED;

static bool boardHasLedStrip() {
  return M5.getBoard() == m5::board_t::board_M5Stack && M5.Led.isEnabled();
}

/** Reconfigures the board's LED strip driver to address all ten LEDs of both bars. */
static void expandStripToTenLeds() {
  auto* strip = static_cast<m5::LED_Strip_Class*>(M5.Led.getLedInstancePtr());
  auto stripConfig = strip->getConfig();
  stripConfig.led_count = TOTAL_LEDS;
  strip->setConfig(stripConfig);
}

void begin() {
  if (!settings::LED_BARS_ENABLED || !boardHasLedStrip()) return;
  expandStripToTenLeds();
  M5.Led.setAutoDisplay(false);
  available = M5.Led.begin();
  if (available) M5.Led.setBrightness(settings::LED_BRIGHTNESS);
}

bool isAvailable() { return available; }
bool isOn() { return switchedOn; }
void toggle() { switchedOn = !switchedOn; }

static Rgb heatColor(float level) {
  if (level > settings::CRITICAL_LEVEL) return LED_RED;
  if (level > settings::WARM_LEVEL) return LED_WARM;
  return LED_AMBER;
}

/** Returns a slowly pulsing dim grey used while the PC link is down. */
static Rgb offlinePulse() {
  uint8_t level = 25 + 20 * sinf(animation::phase * 0.03f);
  return {level, level, level};
}

/**
 * Returns the colour of one LED in a meter: lit LEDs (20 % load each) breathe in
 * the heat colour at a load-dependent rate, unlit ones glow faintly.
 */
static Rgb meterColor(Metric load, Metric temperature, int position, float breathingOffset) {
  const float* level = stats::current().smoothed;
  int litCount = (int)(level[load] / 20 + 0.99f);
  float breathing = 0.8f + 0.2f * sinf(animation::phase * (0.03f + level[load] / 100 * 0.12f) + breathingOffset);
  Rgb base = heatColor(largerOf(level[load], level[temperature]));
  return base.scaled(position < litCount ? breathing : UNLIT_GLOW);
}

/** Returns an LED colour, applying in order: switched off, unlinked pulse, alarm flash, normal meter. */
static Rgb colorFor(Metric load, Metric temperature, int position, float breathingOffset, bool linked) {
  if (!switchedOn) return LED_OFF;
  if (!linked) return offlinePulse();
  if (alert::state().alarmActive) return animation::alarmFlashVisible() ? LED_RED : LED_OFF;
  return meterColor(load, temperature, position, breathingOffset);
}

/** Sets the five LEDs of one bar; the bar stays dark when its metric is not reported. */
static void paintSide(int firstLed, Metric load, Metric temperature, float breathingOffset, bool linked) {
  const bool metricMissing = linked && !stats::current().isAvailable(load);
  for (int position = 0; position < LEDS_PER_SIDE; position++) {
    Rgb color = metricMissing ? LED_OFF : colorFor(load, temperature, position, breathingOffset, linked);
    M5.Led.setColor(firstLed + position, color.red, color.green, color.blue);
  }
}

void update(uint32_t now, bool linked) {
  static uint32_t lastRefresh = 0;
  if (!available || now - lastRefresh < REFRESH_INTERVAL_MS) return;
  lastRefresh = now;
  int cpuSide = settings::LED_SWAP_SIDES ? LEDS_PER_SIDE : 0;
  int gpuSide = settings::LED_SWAP_SIDES ? 0 : LEDS_PER_SIDE;
  paintSide(cpuSide, CPU_LOAD, CPU_TEMP, 0, linked);
  paintSide(gpuSide, GPU_LOAD, GPU_TEMP, 1, linked);
  M5.Led.display();
}

}
