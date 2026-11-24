/**
 * @file temporal_loom.ino
 * @brief Temporal Loom: a PC system monitor for the M5Stack Fire.
 *
 * Receives JSON stat lines from the PC companion over USB serial and shows them
 * as an animated loom with history and detail screens, an over-limit alarm and
 * the side LED bars. Each loop reads serial input, updates state and animation,
 * then renders a frame.
 */
#include <M5Unified.h>
#include "src/config/settings.h"
#include "src/core/alert.h"
#include "src/core/animation.h"
#include "src/core/frame_context.h"
#include "src/core/pc_link.h"
#include "src/core/stats_store.h"
#include "src/display/canvas.h"
#include "src/display/theme.h"
#include "src/display/effects/boot_sequence.h"
#include "src/display/effects/sparks.h"
#include "src/display/screens/screen_router.h"
#include "src/hardware/led_bars.h"
#include "src/input/navigation.h"

/** Shows a fatal error message on a black screen and stops the program. */
void haltWithMessage(const char* message) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.println(message);
  while (true) delay(1000);
}

/** Initialises the hardware, serial link, theme, frame buffer and LED bars, then starts the boot sequence. */
void setup() {
  M5.begin(M5.config());
  pc_link::begin();
  M5.Display.setBrightness(settings::SCREEN_BRIGHTNESS);
  M5.Speaker.setVolume(settings::SPEAKER_VOLUME);
  theme::init();
  if (!canvas::begin()) haltWithMessage("Not enough memory for the frame buffer");
  led_bars::begin();
  boot_sequence::begin();
  randomSeed(esp_random());
}

/** Runs one frame: polls serial, updates stats, alarm, input and animation, refreshes the LEDs and renders. */
void loop() {
  M5.update();
  const uint32_t now = millis();
  pc_link::poll(now);

  const FrameContext frame = {now, pc_link::isConnected(now)};
  const float elapsedFrames = animation::measureElapsedFrames(now);
  const bool onLoomScreen = navigation::current() == Screen::Loom;

  stats::update(elapsedFrames, now, frame.linked);
  alert::update(now, frame.linked);
  navigation::handleButtons(now);
  animation::advance(elapsedFrames, alert::state().alarmActive && onLoomScreen);
  sparks::update(elapsedFrames, frame.linked && onLoomScreen);
  boot_sequence::update(elapsedFrames);
  led_bars::update(now, frame.linked);

  canvas::render([&] {
    if (boot_sequence::isRunning()) boot_sequence::draw();
    else screen_router::draw(frame);
  });
  animation::finishFrame();
}
