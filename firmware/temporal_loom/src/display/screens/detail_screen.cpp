/**
 * @file detail_screen.cpp
 * @brief Row-based text listing of exact values and device status.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "detail_screen.h"
#include "../canvas.h"
#include "../layout.h"
#include "../theme.h"
#include "../../core/alert.h"
#include "../../core/stats_store.h"
#include "../../hardware/led_bars.h"

namespace detail_screen {

static int rowY(int row) { return layout::DETAIL_FIRST_ROW_Y + row * layout::DETAIL_ROW_SPACING; }

static void drawRow(int row, const char* label, const char* value, uint16_t color) {
  canvas::drawText(label, layout::DETAIL_LABEL_X, rowY(row), theme::amberDim, 2, textdatum_t::top_left);
  canvas::drawText(value, layout::DETAIL_VALUE_X, rowY(row), color, 2, textdatum_t::top_left);
}

static void formatTemperature(char* out, size_t size, Metric temperature) {
  const SystemStats& live = stats::current();
  if (live.isAvailable(temperature)) snprintf(out, size, "%dC", (int)(live.received[temperature] + 0.5f));
  else strlcpy(out, "--", size);
}

/** Draws a processor row with load and temperature, coloured by the hotter reading (grey when unlinked). */
static void drawProcessorRow(int row, const char* label, Metric load, Metric temperature, bool linked) {
  const SystemStats& live = stats::current();
  char degrees[8], value[24];
  formatTemperature(degrees, sizeof(degrees), temperature);
  snprintf(value, sizeof(value), "%3d%%  %s", (int)(live.received[load] + 0.5f), degrees);
  uint16_t color = linked ? theme::heatColor(largerOf(live.received[load], live.received[temperature]))
                          : theme::offlineGrey;
  drawRow(row, label, value, color);
}

/** Draws a memory row with usage percent and used/total GB, or "--" when not reported. */
static void drawMemoryRow(int row, const char* label, Metric usage, float usedGb, float totalGb, uint16_t color) {
  if (!stats::current().isAvailable(usage)) {
    drawRow(row, label, "--", theme::amberDim);
    return;
  }
  char value[24];
  snprintf(value, sizeof(value), "%3d%%  %.1f/%.0fG", (int)(stats::current().received[usage] + 0.5f), usedGb, totalGb);
  drawRow(row, label, value, color);
}

void draw(const FrameContext& frame) {
  const SystemStats& live = stats::current();
  uint16_t liveColor = frame.linked ? theme::amber : theme::offlineGrey;
  char processName[19];
  strlcpy(processName, live.topProcessName, sizeof(processName));

  drawProcessorRow(0, "CPU", CPU_LOAD, CPU_TEMP, frame.linked);
  if (live.hasGpu()) drawProcessorRow(1, "GPU", GPU_LOAD, GPU_TEMP, frame.linked);
  else drawRow(1, "GPU", "NOT FOUND", theme::amberDim);
  drawMemoryRow(2, "RAM", RAM_USAGE, live.ramUsedGb, live.ramTotalGb, liveColor);
  drawMemoryRow(3, "VRAM", VRAM_USAGE, live.vramUsedGb, live.vramTotalGb, liveColor);
  drawRow(4, "TOP", processName, liveColor);
  drawRow(5, "LINK", frame.linked ? "ACTIVE" : "SEVERED", frame.linked ? theme::amber : theme::alarmRed);
  drawRow(6, "SOUND", alert::isMuted() ? "MUTED" : "ON", theme::amber);
  if (led_bars::isAvailable()) drawRow(7, "LEDS", led_bars::isOn() ? "ON" : "OFF", theme::amber);
}

}
