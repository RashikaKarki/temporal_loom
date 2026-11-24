#pragma once

/**
 * @file led_bars.h
 * @brief The M5Stack Fire's two side LED bars as CPU and GPU meters.
 *
 * Each bar has five LEDs, each representing 20 % of load, coloured by the
 * hotter of load and temperature. The bars flash red during an alarm and
 * pulse grey while the PC link is down.
 */
#include <stdint.h>

namespace led_bars {

/**
 * Configures the 10-LED strip when settings::LED_BARS_ENABLED is set and the
 * board is an M5Stack with LEDs; otherwise the bars stay unavailable.
 */
void begin();
/** Returns true when the LED bars were initialised successfully. */
bool isAvailable();
/** Returns true when the bars are switched on by the user. */
bool isOn();
/** Switches the bars on or off (holding the middle button). */
void toggle();
/**
 * Repaints both bars, at most every 40 ms.
 * @param now    Current time in ms (millis()).
 * @param linked Whether the PC link is up.
 */
void update(uint32_t now, bool linked);

}
