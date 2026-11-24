/**
 * @file animation.cpp
 * @brief Frame timing, ring rotation, alarm shake and blink timers.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "animation.h"
#include "stats_store.h"

namespace animation {

float phase = 0;
float cpuRingAngle = 0;
float gpuRingAngle = 0;
uint32_t frameCount = 0;
int shakeOffsetX = 0;
int shakeOffsetY = 0;

static constexpr float MS_PER_REFERENCE_FRAME = 16.667f;
static constexpr float MAX_ELAPSED_FRAMES = 4;
static constexpr float IDLE_SPIN = 0.008f;
static constexpr float FULL_LOAD_SPIN = 0.11f;
static constexpr uint32_t ALARM_CYCLE_FRAMES = 36;

float measureElapsedFrames(uint32_t now) {
  static uint32_t previous = now;
  float elapsed = (now - previous) / MS_PER_REFERENCE_FRAME;
  previous = now;
  return elapsed > MAX_ELAPSED_FRAMES ? MAX_ELAPSED_FRAMES : elapsed;
}

/** Returns the ring rotation speed in radians per reference frame for a load in percent. */
static float spinSpeed(float loadPercent) { return IDLE_SPIN + loadPercent / 100 * FULL_LOAD_SPIN; }

void advance(float elapsedFrames, bool shakeAllowed) {
  const float* level = stats::current().smoothed;
  phase += elapsedFrames;
  cpuRingAngle += spinSpeed(level[CPU_LOAD]) * elapsedFrames;
  gpuRingAngle -= spinSpeed(level[GPU_LOAD]) * elapsedFrames;

  const bool shakingNow = shakeAllowed && (frameCount % ALARM_CYCLE_FRAMES) < 6;
  shakeOffsetX = shakingNow ? random(-2, 3) : 0;
  shakeOffsetY = shakingNow ? random(-1, 2) : 0;
}

void finishFrame() { frameCount++; }

bool alarmFlashVisible() { return (frameCount % ALARM_CYCLE_FRAMES) < 20; }

bool slowBlinkVisible() { return (frameCount / 30) % 2 == 0; }

bool cursorBlinkVisible() { return (frameCount / 15) % 2 == 0; }

}
