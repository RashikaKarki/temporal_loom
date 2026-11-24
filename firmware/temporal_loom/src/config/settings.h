#pragma once

/**
 * @file settings.h
 * @brief User-editable configuration for the Temporal Loom firmware.
 *
 * Every tunable value lives here so behaviour can be adjusted without touching
 * the rest of the code. Rebuild and upload the sketch after changing a value.
 */
#include <stdint.h>

namespace settings {

/**
 * Serial baud rate used to talk to the PC companion.
 * Must match the rate configured on the PC side. Default 115200.
 */
constexpr uint32_t SERIAL_BAUD = 115200;
/**
 * Time in ms without a valid stats line before the link is considered lost
 * and the display switches to the "TIMELINE SEVERED" state.
 * Should be comfortably longer than the PC send interval; sensible range 1000-10000.
 */
constexpr uint32_t LINK_TIMEOUT_MS = 3000;
/**
 * Interval in ms between samples written to the usage history.
 * The history holds 160 samples, so the recorded window is 160 x this value
 * (250 ms gives the 40 s window shown by the "LAST 40 SEC" label on the history screen;
 * that label does not update automatically). Sensible range 100-2000.
 */
constexpr uint32_t HISTORY_SAMPLE_MS = 250;

/** CPU load in percent above which the CPU is considered critical. Range 0-100. */
constexpr float CPU_LOAD_LIMIT = 85;
/** CPU temperature in °C above which the CPU is considered critical. Sensible range 60-100. */
constexpr float CPU_TEMP_LIMIT = 85;
/** GPU load in percent above which the GPU is considered critical. Range 0-100. */
constexpr float GPU_LOAD_LIMIT = 90;
/** GPU temperature in °C above which the GPU is considered critical. Sensible range 60-100. */
constexpr float GPU_TEMP_LIMIT = 83;
/**
 * Time in ms a reading must stay over its limit before the alarm activates
 * (siren, flashing banner, shaking loom, red LEDs). Filters out short spikes.
 * Sensible range 0-10000.
 */
constexpr uint32_t ALARM_DELAY_MS = 3000;
/** Interval in ms between repeated alarm beeps while the alarm stays active. Sensible range 2000-60000. */
constexpr uint32_t ALARM_REPEAT_MS = 10000;

/**
 * Level above which values are drawn in the warm amber colour.
 * Applies to both percentages and °C readings on screen and on the LED bars. Range 0-100.
 */
constexpr float WARM_LEVEL = 65;
/**
 * Level above which values are drawn in alarm red.
 * Applies to both percentages and °C readings; keep it above WARM_LEVEL. Range 0-100.
 */
constexpr float CRITICAL_LEVEL = 85;

/** LCD backlight brightness. Range 0 (off) to 255 (maximum). */
constexpr uint8_t SCREEN_BRIGHTNESS = 160;
/** Speaker volume for alarm beeps and button clicks. Range 0 (silent) to 255 (maximum). */
constexpr uint8_t SPEAKER_VOLUME = 90;
/** Time in ms the button hint labels stay visible after any button press. Sensible range 1000-10000. */
constexpr uint32_t BUTTON_HINT_MS = 3000;

/**
 * Enables the two side LED bars of the M5Stack Fire.
 * When false the LEDs are never initialised and cannot be toggled with the middle button.
 */
constexpr bool LED_BARS_ENABLED = true;
/** Global LED bar brightness. Range 0-255; low values (20-120) avoid glare and reduce power draw. */
constexpr uint8_t LED_BRIGHTNESS = 60;
/** Swaps which LED bar shows the CPU and which shows the GPU. */
constexpr bool LED_SWAP_SIDES = false;

}
