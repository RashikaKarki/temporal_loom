/**
 * @file history_screen.cpp
 * @brief Usage history graph with grid and legend.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "history_screen.h"
#include "../canvas.h"
#include "../layout.h"
#include "../theme.h"
#include "../../core/stats_store.h"
#include "../../input/navigation.h"

namespace history_screen {

using canvas::toBand;

static constexpr int GRAPH_WIDTH = canvas::WIDTH - layout::GRAPH_LEFT - 6;
static constexpr int GRAPH_HEIGHT = layout::GRAPH_BOTTOM - layout::GRAPH_TOP;

/** Maps a history index to a graph x position so the newest sample is always at the right edge. */
static int graphX(int sampleIndex, int length) {
  return layout::GRAPH_LEFT + (UsageHistory::CAPACITY - length + sampleIndex) * GRAPH_WIDTH / (UsageHistory::CAPACITY - 1);
}

static int graphY(uint8_t percent) { return layout::GRAPH_BOTTOM - percent * GRAPH_HEIGHT / 100; }

static void drawGrid() {
  char label[6];
  for (int percent = 0; percent <= 100; percent += 25) {
    int y = layout::GRAPH_BOTTOM - GRAPH_HEIGHT * percent / 100;
    canvas::surface().drawFastHLine(layout::GRAPH_LEFT, toBand(y), GRAPH_WIDTH, theme::emptySegment);
    snprintf(label, sizeof(label), "%d", percent);
    canvas::drawText(label, layout::GRAPH_LEFT - 4, y, theme::amberDim, 1, textdatum_t::middle_right);
  }
}

/** Draws one history series as connected line segments, optionally dashed or two pixels thick. */
static void drawSeries(HistorySeries series, uint16_t color, bool dashed, bool bold) {
  M5Canvas& surface = canvas::surface();
  const UsageHistory& history = stats::history();
  for (int i = 1; i < history.length; i++) {
    if (dashed && (i / 2) % 2) continue;
    int startX = graphX(i - 1, history.length), endX = graphX(i, history.length);
    int startY = graphY(history.sample(series, i - 1)), endY = graphY(history.sample(series, i));
    surface.drawLine(startX, toBand(startY), endX, toBand(endY), color);
    if (bold) surface.drawLine(startX, toBand(startY - 1), endX, toBand(endY - 1), color);
  }
}

static void drawLegend() {
  canvas::drawText("CPU", 8, layout::FOOTER_TEXT_Y, theme::amber, 1, textdatum_t::top_left);
  uint16_t gpuLabelColor = stats::current().hasGpu() ? theme::amberPale : theme::amberDim;
  canvas::drawText("GPU", 40, layout::FOOTER_TEXT_Y, gpuLabelColor, 1, textdatum_t::top_left);
  canvas::drawText("RAM", 72, layout::FOOTER_TEXT_Y, theme::ramAmber, 1, textdatum_t::top_left);
  canvas::drawText("LAST 40 SEC", layout::METER_RIGHT, layout::FOOTER_TEXT_Y, theme::amberDim, 1,
                   textdatum_t::top_right);
}

void draw(const FrameContext& frame) {
  auto colorWhenLinked = [&](uint16_t color) { return frame.linked ? color : theme::offlineGreyDark; };
  drawGrid();
  drawSeries(HISTORY_RAM, colorWhenLinked(theme::ramAmber), false, false);
  if (stats::current().hasGpu()) drawSeries(HISTORY_GPU, colorWhenLinked(theme::amberPale), true, false);
  drawSeries(HISTORY_CPU, colorWhenLinked(theme::amber), false, true);
  if (!navigation::hintsVisible(frame.now)) drawLegend();
}

}
