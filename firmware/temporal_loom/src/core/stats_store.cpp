/**
 * @file stats_store.cpp
 * @brief JSON parsing, value smoothing and usage history recording.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include <ArduinoJson.h>
#include "stats_store.h"
#include "../config/settings.h"

float largerOf(float a, float b) { return a > b ? a : b; }

static uint8_t asPercentByte(float value) { return (uint8_t)constrain((int)value, 0, 100); }

void UsageHistory::push(uint8_t cpu, uint8_t gpu, uint8_t ram) {
  if (length < CAPACITY) {
    length++;
  } else {
    for (int series = 0; series < HISTORY_SERIES_COUNT; series++) {
      memmove(samples[series], samples[series] + 1, CAPACITY - 1);
    }
  }
  samples[HISTORY_CPU][length - 1] = cpu;
  samples[HISTORY_GPU][length - 1] = gpu;
  samples[HISTORY_RAM][length - 1] = ram;
}

namespace stats {

static SystemStats liveStats;
static UsageHistory usageHistory;
static uint32_t lastHistorySample = 0;

const SystemStats& current() { return liveStats; }
const UsageHistory& history() { return usageHistory; }

bool applyJson(const char* line) {
  JsonDocument message;
  if (deserializeJson(message, line)) return false;

  liveStats.received[CPU_LOAD] = message["cpu"] | 0.0f;
  liveStats.received[CPU_TEMP] = message["ctmp"] | UNAVAILABLE;
  liveStats.received[GPU_LOAD] = message["gpu"] | UNAVAILABLE;
  liveStats.received[GPU_TEMP] = message["gtmp"] | UNAVAILABLE;
  liveStats.received[RAM_USAGE] = message["ram"] | 0.0f;
  liveStats.received[VRAM_USAGE] = message["vram"] | UNAVAILABLE;
  liveStats.ramUsedGb = message["ramu"] | 0.0f;
  liveStats.ramTotalGb = message["ramt"] | 0.0f;
  liveStats.vramUsedGb = message["vramu"] | 0.0f;
  liveStats.vramTotalGb = message["vramt"] | 0.0f;
  liveStats.topProcessPercent = message["pp"] | 0.0f;
  strlcpy(liveStats.topProcessName, message["proc"] | "--", sizeof(liveStats.topProcessName));
  strlcpy(liveStats.clockText, message["clk"] | "--:--", sizeof(liveStats.clockText));
  return true;
}

/**
 * Eases each smoothed value 8 % of the way to its received value per reference
 * frame and lets the recent peak fall by 0.03 percentage points per frame.
 */
static void smoothTowardsReceived(float elapsedFrames) {
  const float easing = 1 - powf(0.92f, elapsedFrames);
  const float peakFallPerFrame = 0.03f;
  for (int metric = 0; metric < METRIC_COUNT; metric++) {
    float target = largerOf(liveStats.received[metric], 0);
    liveStats.smoothed[metric] += (target - liveStats.smoothed[metric]) * easing;
    liveStats.recentPeak[metric] =
        largerOf(liveStats.recentPeak[metric] - peakFallPerFrame * elapsedFrames, liveStats.smoothed[metric]);
  }
}

/** Pushes the smoothed CPU, GPU and RAM usage into the history every settings::HISTORY_SAMPLE_MS. */
static void sampleHistory(uint32_t now) {
  if (now - lastHistorySample < settings::HISTORY_SAMPLE_MS) return;
  lastHistorySample = now;
  usageHistory.push(asPercentByte(liveStats.smoothed[CPU_LOAD]), asPercentByte(liveStats.smoothed[GPU_LOAD]),
                    asPercentByte(liveStats.smoothed[RAM_USAGE]));
}

void update(float elapsedFrames, uint32_t now, bool linked) {
  smoothTowardsReceived(elapsedFrames);
  if (linked) sampleHistory(now);
}

float busiestLoad() { return largerOf(liveStats.smoothed[CPU_LOAD], liveStats.smoothed[GPU_LOAD]); }

float hottestReading() {
  return largerOf(busiestLoad(), largerOf(liveStats.smoothed[CPU_TEMP], liveStats.smoothed[GPU_TEMP]));
}

}
