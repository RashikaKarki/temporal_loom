/**
 * @file loom_screen.cpp
 * @brief The animated loom, the meter column and the recent-history strip.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "loom_screen.h"
#include "../canvas.h"
#include "../layout.h"
#include "../theme.h"
#include "../effects/sparks.h"
#include "../../core/animation.h"
#include "../../core/stats_store.h"

namespace loom_screen {

using canvas::toBand;

struct Ring {
  int innerRadius;
  int outerRadius;
  int threadCount;
  int direction;
  float angle;
  float load;
  int thickness;
  uint16_t color;
};

/**
 * Draws one curved thread of a ring as a quadratic Bezier from the inner to the
 * outer radius, bent by a load-dependent wobble and drawn 1-3 pixels thick.
 */
static void drawThread(const Ring& ring, int centerX, int centerY, int index) {
  M5Canvas& surface = canvas::surface();
  float baseAngle = ring.angle + index * 2 * PI / ring.threadCount;
  float wobble = sinf(animation::phase * 0.06f + index) * ring.load * 0.5f * ring.direction;
  float middleRadius = (ring.innerRadius + ring.outerRadius) * 0.5f;
  float bendAngle = baseAngle + ring.direction * 0.45f + wobble;
  float endAngle = baseAngle + ring.direction * 0.8f;
  uint16_t shade = theme::blend(theme::background, ring.color, 0.45f + 0.55f * ((index % 3) / 2.0f));

  int startX = centerX + cosf(baseAngle) * ring.innerRadius, startY = centerY + sinf(baseAngle) * ring.innerRadius;
  int bendX = centerX + cosf(bendAngle) * middleRadius, bendY = centerY + sinf(bendAngle) * middleRadius;
  int endX = centerX + cosf(endAngle) * ring.outerRadius, endY = centerY + sinf(endAngle) * ring.outerRadius;

  surface.drawBezier(startX, toBand(startY), bendX, toBand(bendY), endX, toBand(endY), shade);
  if (ring.thickness >= 2) {
    surface.drawBezier(startX + 1, toBand(startY), bendX + 1, toBand(bendY), endX + 1, toBand(endY), shade);
  }
  if (ring.thickness >= 3) {
    surface.drawBezier(startX, toBand(startY + 1), bendX, toBand(bendY + 1), endX, toBand(endY + 1), shade);
  }
}

static void drawRing(const Ring& ring, int centerX, int centerY) {
  for (int index = 0; index < ring.threadCount; index++) drawThread(ring, centerX, centerY, index);
}

/** Returns the thread thickness in pixels (1-3) for a memory usage in percent. */
static int threadThickness(float memoryPercent) { return 1 + (int)(memoryPercent / 40); }

/** Draws the loom rim and divider circles and the 24 rotating tick marks around the rim. */
static void drawFrame(int centerX, int centerY) {
  M5Canvas& surface = canvas::surface();
  const int frameRadii[] = {layout::LOOM_RADIUS, layout::LOOM_RADIUS - 1, layout::LOOM_RADIUS - 5,
                            layout::LOOM_DIVIDER_RADIUS, layout::LOOM_DIVIDER_RADIUS - 1};
  for (int radius : frameRadii) surface.drawCircle(centerX, toBand(centerY), radius, theme::amberDim);
  for (int tick = 0; tick < 24; tick++) {
    float angle = tick * PI / 12 - animation::cpuRingAngle * 0.3f;
    int x = centerX + cosf(angle) * (layout::LOOM_RADIUS + 5);
    int y = centerY + sinf(angle) * (layout::LOOM_RADIUS + 5);
    surface.fillRect(x - 1, toBand(y - 1), 3, 3, theme::amberDim);
  }
}

/** Draws the centre dot, pulsing faster with load and coloured by the hottest reading. */
static void drawPulsingCore(int centerX, int centerY) {
  float pulseSpeed = 0.05f + stats::busiestLoad() / 400;
  int radius = 6 + (int)(sinf(animation::phase * pulseSpeed) * 3);
  canvas::surface().fillCircle(centerX, toBand(centerY), radius, theme::heatColor(stats::hottestReading()));
}

/** Draws the live loom: frame, CPU ring (9 threads), GPU ring (14 threads), sparks and core, shaken during an alarm. */
static void drawLinkedLoom() {
  const float* level = stats::current().smoothed;
  int centerX = layout::LOOM_CENTER_X + animation::shakeOffsetX;
  int centerY = layout::LOOM_CENTER_Y + animation::shakeOffsetY;
  Ring cpuRing = {layout::CPU_RING_INNER, layout::CPU_RING_OUTER, 9, 1, animation::cpuRingAngle,
                  level[CPU_LOAD] / 100, threadThickness(level[RAM_USAGE]),
                  theme::heatColor(largerOf(level[CPU_LOAD], level[CPU_TEMP]))};
  const bool hasGpu = stats::current().hasGpu();
  Ring gpuRing = {layout::GPU_RING_INNER, layout::GPU_RING_OUTER, 14, -1, animation::gpuRingAngle,
                  level[GPU_LOAD] / 100, threadThickness(level[VRAM_USAGE]),
                  hasGpu ? theme::heatColor(largerOf(level[GPU_LOAD], level[GPU_TEMP])) : theme::offlineGreyDark};
  drawFrame(centerX, centerY);
  drawRing(cpuRing, centerX, centerY);
  drawRing(gpuRing, centerX, centerY);
  sparks::draw();
  drawPulsingCore(centerX, centerY);
}

/** Draws the broken, grey loom and the "TIMELINE SEVERED" message shown while unlinked. */
static void drawSeveredLoom() {
  M5Canvas& surface = canvas::surface();
  const int centerX = layout::LOOM_CENTER_X, centerY = layout::LOOM_CENTER_Y, radius = layout::LOOM_RADIUS;
  for (float angle = 0.3f; angle < 5.9f; angle += 0.12f) {
    const bool gapInRim = (int)(angle * 10) % 3 == 0;
    if (gapInRim) continue;
    surface.drawLine(centerX + cosf(angle) * radius, toBand(centerY + sinf(angle) * radius),
                     centerX + cosf(angle + 0.08f) * radius, toBand(centerY + sinf(angle + 0.08f) * radius),
                     theme::offlineGreyDark);
  }
  for (int shard = 0; shard < 6; shard++) {
    float angle = shard * 1.05f + 0.4f;
    int reach = 45 + (shard % 2) * 10;
    surface.drawLine(centerX + cosf(angle) * 24, toBand(centerY + sinf(angle) * 24), centerX + cosf(angle) * reach,
                     toBand(centerY + sinf(angle) * reach), theme::offlineGreyDark);
  }
  uint16_t titleColor = animation::slowBlinkVisible() ? theme::amber : theme::amberDim;
  canvas::drawText("TIMELINE SEVERED", 245, 92, titleColor, 1, textdatum_t::middle_center);
  canvas::drawText("awaiting PC link", 245, 112, theme::amberDim, 1, textdatum_t::middle_center);
  canvas::drawText("start the PC companion", 245, 126, theme::amberDim, 1, textdatum_t::middle_center);
}

static void drawEmptySegments(int y) {
  for (int segment = 0; segment < layout::METER_SEGMENTS; segment++) {
    canvas::surface().fillRect(layout::METER_LEFT + segment * layout::METER_SEGMENT_PITCH, toBand(y + 13), 10, 7,
                               theme::emptySegment);
  }
}

static void drawUnavailableMeterRow(const char* label, int y) {
  canvas::drawText(label, layout::METER_LEFT, y, theme::amberDim, 1, textdatum_t::top_left);
  canvas::drawText("N/A", layout::METER_RIGHT, y - 4, theme::amberDim, 2, textdatum_t::top_right);
  drawEmptySegments(y);
}

/**
 * Draws one meter row: label, optional temperature, percentage, 10-segment bar
 * and the recent-peak marker. Pass METRIC_COUNT as temperature for rows without one.
 */
static void drawMeterRow(const char* label, Metric load, Metric temperature, int y) {
  const SystemStats& live = stats::current();
  if (!live.isAvailable(load)) {
    drawUnavailableMeterRow(label, y);
    return;
  }
  float value = live.smoothed[load];
  char text[8];
  canvas::drawText(label, layout::METER_LEFT, y, theme::amber, 1, textdatum_t::top_left);
  if (temperature != METRIC_COUNT && live.isAvailable(temperature)) {
    float degrees = live.smoothed[temperature];
    snprintf(text, sizeof(text), "%dC", (int)(degrees + 0.5f));
    canvas::drawText(text, layout::METER_TEMPERATURE_X, y, theme::heatColor(degrees), 1, textdatum_t::top_left);
  }
  snprintf(text, sizeof(text), "%d%%", (int)(value + 0.5f));
  canvas::drawText(text, layout::METER_RIGHT, y - 4, theme::heatColor(value), 2, textdatum_t::top_right);

  M5Canvas& surface = canvas::surface();
  int litSegments = (int)(value / 10 + 0.5f);
  for (int segment = 0; segment < layout::METER_SEGMENTS; segment++) {
    uint16_t color = segment < litSegments ? theme::heatColor(value) : theme::emptySegment;
    surface.fillRect(layout::METER_LEFT + segment * layout::METER_SEGMENT_PITCH, toBand(y + 13), 10, 7, color);
  }
  int meterWidth = layout::METER_SEGMENTS * layout::METER_SEGMENT_PITCH;
  int peakX = layout::METER_LEFT + (int)(min(live.recentPeak[load], 99.5f) / 100 * meterWidth) - 1;
  surface.fillRect(peakX, toBand(y + 11), 2, 11, theme::amberPale);
}

static void drawMeters() {
  drawMeterRow("CPU", CPU_LOAD, CPU_TEMP, layout::METER_ROW_CPU);
  drawMeterRow("GPU", GPU_LOAD, GPU_TEMP, layout::METER_ROW_GPU);
  drawMeterRow("RAM", RAM_USAGE, METRIC_COUNT, layout::METER_ROW_RAM);
  drawMeterRow("VRAM", VRAM_USAGE, METRIC_COUNT, layout::METER_ROW_VRAM);
}

static int stripY(uint8_t percent) { return layout::STRIP_BASELINE_Y - 1 - percent * layout::STRIP_HEIGHT / 100; }

/** Draws the most recent CPU and GPU history as a small strip, two pixels per sample, newest on the right. */
static void drawTimelineStrip() {
  M5Canvas& surface = canvas::surface();
  const UsageHistory& history = stats::history();
  surface.drawFastHLine(0, toBand(layout::STRIP_BASELINE_Y), canvas::WIDTH, theme::emptySegment);
  const HistorySeries plotted[] = {HISTORY_CPU, HISTORY_GPU};
  for (HistorySeries series : plotted) {
    if (series == HISTORY_GPU && !stats::current().hasGpu()) continue;
    const bool dashed = series == HISTORY_GPU;
    uint16_t color = dashed ? theme::amberPale : theme::amber;
    for (int i = 1; i < history.length; i++) {
      if (dashed && (i / 2) % 2) continue;
      int endX = canvas::WIDTH - 2 - (history.length - 1 - i) * 2;
      surface.drawLine(endX - 2, toBand(stripY(history.sample(series, i - 1))), endX,
                       toBand(stripY(history.sample(series, i))), color);
    }
  }
  canvas::drawText("CPU", 4, layout::STRIP_LABEL_Y, theme::amber, 1, textdatum_t::top_left);
  uint16_t gpuLabelColor = stats::current().hasGpu() ? theme::amberPale : theme::amberDim;
  canvas::drawText("GPU", 30, layout::STRIP_LABEL_Y, gpuLabelColor, 1, textdatum_t::top_left);
}

void draw(const FrameContext& frame) {
  if (!frame.linked) {
    drawSeveredLoom();
    return;
  }
  drawLinkedLoom();
  drawMeters();
  drawTimelineStrip();
}

}
