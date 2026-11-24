#pragma once

/**
 * @file alert.h
 * @brief Over-limit detection and the audible alarm.
 *
 * Compares smoothed CPU/GPU load and temperature, and RAM usage, against the limits in settings.h,
 * activates the alarm once a reading has stayed over its limit for ALARM_DELAY_MS,
 * beeps every ALARM_REPEAT_MS, and handles muting and one-off silencing.
 */
#include <stdint.h>

/** Snapshot of the current alert evaluation, refreshed by alert::update(). */
struct AlertState {
  /** CPU load or CPU temperature is above its configured limit. */
  bool cpuCritical = false;
  /** GPU load or GPU temperature is above its configured limit. */
  bool gpuCritical = false;
  /** System RAM usage is above its configured limit. */
  bool ramCritical = false;
  /** Any of CPU, GPU or RAM is over its limit while the PC link is up. */
  bool critical = false;
  /** Critical has persisted for at least ALARM_DELAY_MS; drives visual and audible alarm effects. */
  bool alarmActive = false;
};

namespace alert {

/**
 * Re-evaluates thresholds, tracks how long the critical condition has lasted and
 * plays the alarm beep when due. Call once per loop.
 * @param now    Current time in ms (millis()).
 * @param linked Whether the PC link is up; nothing is critical while disconnected.
 * @note Playing a beep blocks for roughly 140 ms.
 */
void update(uint32_t now, bool linked);
/** Returns the most recent alert evaluation. */
const AlertState& state();
/** Returns true when the alarm sound is muted indefinitely by the user. */
bool isMuted();
/**
 * Returns true when the middle button would silence the current critical episode
 * (critical, not already silenced and not muted) rather than toggle the mute.
 */
bool canSilence();
/**
 * Handles a click of the middle button. During a critical episode it silences the
 * beeps until readings return below their limits; otherwise it toggles the
 * persistent mute. Plays a short click unless the result is muted.
 */
void handleMuteButton();

}
