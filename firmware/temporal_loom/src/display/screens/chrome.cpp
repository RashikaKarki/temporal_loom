/**
 * @file chrome.cpp
 * @brief Header bar, alarm banner, button hints and top-process footer.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "chrome.h"
#include "../canvas.h"
#include "../layout.h"
#include "../theme.h"
#include "../../core/alert.h"
#include "../../core/animation.h"
#include "../../core/stats_store.h"
#include "../../input/navigation.h"

namespace chrome {

/** Returns the header status: link lost, which processor is critical, or stable (with "M" when muted). */
static const char* statusText(const FrameContext& frame) {
  const AlertState& alarm = alert::state();
  if (!frame.linked) return "NO SIGNAL";
  if (alarm.alarmActive) {
    if (alarm.cpuCritical && alarm.gpuCritical) return "ALL CRIT";
    return alarm.cpuCritical ? "CPU CRIT" : "GPU CRIT";
  }
  return alert::isMuted() ? "STABLE  M" : "STABLE";
}

static uint16_t headerColor(const FrameContext& frame) {
  if (!frame.linked) return theme::offlineGrey;
  return alert::state().alarmActive ? theme::alarmRed : theme::amber;
}

/** Copies the PC clock text, hiding the colon every other 500 ms. */
static void blinkingClock(char* out, size_t size, uint32_t now) {
  strlcpy(out, stats::current().clockText, size);
  const bool colonHidden = (now / 500) % 2 == 1;
  if (colonHidden && out[2] == ':') out[2] = ' ';
}

void drawHeader(const FrameContext& frame) {
  canvas::surface().fillRect(0, canvas::toBand(0), canvas::WIDTH, layout::HEADER_HEIGHT, headerColor(frame));
  char clock[8];
  blinkingClock(clock, sizeof(clock), frame.now);
  canvas::drawText("TEMPORAL LOOM", 5, layout::HEADER_TEXT_Y, theme::background, 1, textdatum_t::top_left);
  canvas::drawText(clock, layout::HEADER_CLOCK_X, layout::HEADER_TEXT_Y, theme::background, 1, textdatum_t::top_center);
  canvas::drawText(statusText(frame), canvas::WIDTH - 5, layout::HEADER_TEXT_Y, theme::background, 1,
                   textdatum_t::top_right);
}

static void drawAlarmBanner() {
  canvas::surface().fillRect(0, canvas::toBand(layout::FOOTER_TOP), canvas::WIDTH, canvas::HEIGHT - layout::FOOTER_TOP,
                             theme::alarmRed);
  canvas::drawText("LOOM AT CAPACITY", canvas::WIDTH / 2, 225, theme::background, 2, textdatum_t::middle_center);
}

static const char* middleButtonLabel() {
  if (alert::canSilence()) return "SILENCE";
  return alert::isMuted() ? "UNMUTE" : "MUTE";
}

/** Draws the labels for the three buttons above their positions on the bezel. */
static void drawButtonHints() {
  M5Canvas& surface = canvas::surface();
  surface.fillRect(0, canvas::toBand(layout::HINT_TOP), canvas::WIDTH, canvas::HEIGHT - layout::HINT_TOP,
                   theme::background);
  canvas::drawText(navigation::nameOf(navigation::previous()), layout::HINT_LEFT_X, layout::HINT_TEXT_Y, theme::amber,
                   1, textdatum_t::middle_center);
  canvas::drawText(middleButtonLabel(), layout::HINT_MIDDLE_X, layout::HINT_TEXT_Y, theme::amber, 1,
                   textdatum_t::middle_center);
  canvas::drawText(navigation::nameOf(navigation::next()), layout::HINT_RIGHT_X, layout::HINT_TEXT_Y, theme::amber, 1,
                   textdatum_t::middle_center);
  for (int button = 0; button < 3; button++) {
    surface.fillRect(45 + button * 95, canvas::toBand(236), 40, 2, theme::amberDim);
  }
}

static void drawTopProcess() {
  const SystemStats& live = stats::current();
  char percent[8];
  snprintf(percent, sizeof(percent), "%d%%", (int)(live.topProcessPercent + 0.5f));
  canvas::drawText("PRIME VARIANT", 5, layout::FOOTER_TEXT_Y, theme::amberDim, 1, textdatum_t::top_left);
  canvas::drawText(live.topProcessName, 96, layout::FOOTER_TEXT_Y, theme::amber, 1, textdatum_t::top_left);
  canvas::drawText(percent, layout::METER_RIGHT, layout::FOOTER_TEXT_Y, theme::amber, 1, textdatum_t::top_right);
}

void drawFooter(const FrameContext& frame) {
  const bool alarmActive = alert::state().alarmActive;
  if (alarmActive && animation::alarmFlashVisible()) {
    drawAlarmBanner();
  } else if (navigation::hintsVisible(frame.now)) {
    drawButtonHints();
  } else if (navigation::current() == Screen::Loom && frame.linked && !alarmActive) {
    drawTopProcess();
  }
}

}
