#pragma once

/**
 * @file stats_store.h
 * @brief Owner of the live system statistics and the usage history.
 *
 * Parses JSON stats lines, smooths the values for display and records the
 * history sampled every settings::HISTORY_SAMPLE_MS.
 */
#include "metrics.h"

/** Returns the larger of two values. */
float largerOf(float a, float b);

namespace stats {

/** Returns the live statistics. */
const SystemStats& current();
/** Returns the recorded usage history. */
const UsageHistory& history();
/**
 * Parses one JSON stats line and stores its values.
 * Recognised keys: cpu, ctmp, gpu, gtmp, ram, vram (percent / °C), ramu, ramt,
 * vramu, vramt (GB), pp (top process percent), proc (top process name) and
 * clk ("HH:MM"). Missing keys reset to their defaults, so a missing ctmp, gpu,
 * gtmp or vram marks that metric unavailable.
 * @param line Null-terminated JSON text.
 * @return False if the line is not valid JSON; the stored stats are then unchanged.
 */
bool applyJson(const char* line);
/**
 * Eases the smoothed values towards the received ones, decays the recent peaks
 * and, while linked, appends a history sample when one is due.
 * @param elapsedFrames Elapsed time in reference frames (1 = 16.667 ms).
 * @param now           Current time in ms (millis()).
 * @param linked        Whether the PC link is up; history is paused while disconnected.
 */
void update(float elapsedFrames, uint32_t now, bool linked);
/** Returns the higher of the smoothed CPU and GPU load, in percent. */
float busiestLoad();
/**
 * Returns the highest of the smoothed CPU/GPU loads (percent) and temperatures (°C),
 * used to pick the heat colour of the loom core.
 */
float hottestReading();

}
