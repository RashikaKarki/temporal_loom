/**
 * @file alert.cpp
 * @brief Threshold evaluation, alarm timing and beeping.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "alert.h"
#include "stats_store.h"
#include "../config/settings.h"

namespace alert {

static AlertState alertState;
static uint32_t criticalSince = 0;
static uint32_t lastBeepAt = 0;
static bool muted = false;
static bool silencedUntilCalm = false;

/** Plays the alarm double beep (1760 Hz); blocks for about 140 ms. */
static void playAlarmBeep() {
  M5.Speaker.tone(1760, 90);
  delay(140);
  M5.Speaker.tone(1760, 90);
}

static void playClick() { M5.Speaker.tone(2400, 30); }

/** Compares the smoothed readings with the configured limits; nothing is critical while unlinked. */
static void evaluateThresholds(bool linked) {
  const float* level = stats::current().smoothed;
  alertState.cpuCritical = level[CPU_LOAD] > settings::CPU_LOAD_LIMIT || level[CPU_TEMP] > settings::CPU_TEMP_LIMIT;
  alertState.gpuCritical = level[GPU_LOAD] > settings::GPU_LOAD_LIMIT || level[GPU_TEMP] > settings::GPU_TEMP_LIMIT;
  alertState.critical = linked && (alertState.cpuCritical || alertState.gpuCritical);
}

/**
 * Records when the critical condition began and activates the alarm after
 * settings::ALARM_DELAY_MS. Leaving the critical state clears any silencing.
 */
static void trackCriticalDuration(uint32_t now) {
  if (alertState.critical) {
    if (criticalSince == 0) criticalSince = now;
  } else {
    criticalSince = 0;
    silencedUntilCalm = false;
  }
  alertState.alarmActive = alertState.critical && now - criticalSince >= settings::ALARM_DELAY_MS;
}

/** Beeps immediately when the alarm activates and then every settings::ALARM_REPEAT_MS, unless muted or silenced. */
static void beepIfDue(uint32_t now) {
  if (!alertState.alarmActive) {
    lastBeepAt = 0;
    return;
  }
  const bool soundAllowed = !muted && !silencedUntilCalm;
  const bool beepDue = lastBeepAt == 0 || now - lastBeepAt >= settings::ALARM_REPEAT_MS;
  if (soundAllowed && beepDue) {
    playAlarmBeep();
    lastBeepAt = now;
  }
}

void update(uint32_t now, bool linked) {
  evaluateThresholds(linked);
  trackCriticalDuration(now);
  beepIfDue(now);
}

const AlertState& state() { return alertState; }

bool isMuted() { return muted; }

bool canSilence() { return alertState.critical && !silencedUntilCalm && !muted; }

void handleMuteButton() {
  if (canSilence()) silencedUntilCalm = true;
  else muted = !muted;
  if (!muted) playClick();
}

}
