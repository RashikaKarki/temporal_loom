#pragma once

/**
 * @file metrics.h
 * @brief Data types for the system statistics received from the PC.
 *
 * Defines the metric identifiers, the live statistics record and the fixed-size
 * usage history used by the history graph and the loom timeline strip.
 */
#include <stdint.h>

/**
 * Identifies one received metric. Loads and usages are in percent (0-100),
 * temperatures in °C. METRIC_COUNT is the number of metrics, not a metric.
 */
enum Metric : uint8_t { CPU_LOAD, CPU_TEMP, GPU_LOAD, GPU_TEMP, RAM_USAGE, VRAM_USAGE, METRIC_COUNT };

/** Identifies one series in the usage history. HISTORY_SERIES_COUNT is the number of series. */
enum HistorySeries : uint8_t { HISTORY_CPU, HISTORY_GPU, HISTORY_RAM, HISTORY_SERIES_COUNT };

/** Sentinel stored in SystemStats::received for a metric the PC did not report. */
constexpr float UNAVAILABLE = -1;

/** Latest statistics from the PC together with the smoothed values used for display. */
struct SystemStats {
  /** Raw values from the last stats line, indexed by Metric; UNAVAILABLE when not reported. */
  float received[METRIC_COUNT] = {0, UNAVAILABLE, 0, UNAVAILABLE, 0, 0};
  /** Values eased towards received each frame; unavailable metrics ease towards 0. */
  float smoothed[METRIC_COUNT] = {};
  /** Recent maximum of each smoothed value, decaying slowly; drawn as the meter peak marker. */
  float recentPeak[METRIC_COUNT] = {};
  /** System RAM in use, in GB. */
  float ramUsedGb = 0;
  /** Total system RAM, in GB. */
  float ramTotalGb = 0;
  /** GPU memory in use, in GB. */
  float vramUsedGb = 0;
  /** Total GPU memory, in GB. */
  float vramTotalGb = 0;
  /** CPU usage of the busiest process, in percent. */
  float topProcessPercent = 0;
  /** Name of the busiest process, truncated to fit; "--" when unknown. */
  char topProcessName[24] = "--";
  /** PC wall clock as "HH:MM"; "--:--" when unknown. */
  char clockText[8] = "--:--";

  /** Returns true when the PC reported a value for the metric in its last message. */
  bool isAvailable(Metric metric) const { return received[metric] >= 0; }
  /** Returns true when the PC reports GPU load, i.e. a GPU was detected. */
  bool hasGpu() const { return isAvailable(GPU_LOAD); }
};

/**
 * Rolling history of CPU, GPU and RAM usage in whole percent.
 * Samples are stored oldest first; once full, the oldest sample is dropped on each push.
 */
struct UsageHistory {
  /** Maximum number of samples kept per series. */
  static constexpr int CAPACITY = 160;
  /** Sample storage per series, in percent (0-100); only the first `length` entries are valid. */
  uint8_t samples[HISTORY_SERIES_COUNT][CAPACITY] = {};
  /** Number of valid samples, from 0 up to CAPACITY. */
  int length = 0;

  /** Appends one sample (percent, 0-100) to each series, discarding the oldest when full. */
  void push(uint8_t cpu, uint8_t gpu, uint8_t ram);
  /** Returns the sample at index (0 = oldest) of the given series. */
  uint8_t sample(HistorySeries series, int index) const { return samples[series][index]; }
};
