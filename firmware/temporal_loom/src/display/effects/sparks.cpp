/**
 * @file sparks.cpp
 * @brief Fixed-size spark particle pool emitted from the loom rim.
 */
#include <Arduino.h>
#include <M5Unified.h>
#include "sparks.h"
#include "../canvas.h"
#include "../layout.h"
#include "../theme.h"
#include "../../core/stats_store.h"

namespace sparks {

struct Spark {
  float x, y, velocityX, velocityY, life;
};

static constexpr int MAX_SPARKS = 40;
static constexpr float EMISSION_THRESHOLD = 75;
static constexpr float GRAVITY = 0.04f;
static Spark pool[MAX_SPARKS];
static int activeCount = 0;

/** Spawns a spark at a random point on the loom rim, moving outwards and along the rim. */
static void emitFromRim() {
  float angle = random(628) / 100.0f;
  float speed = 0.8f + random(160) / 100.0f;
  Spark& spark = pool[activeCount++];
  spark.x = layout::LOOM_CENTER_X + cosf(angle) * layout::LOOM_RADIUS;
  spark.y = layout::LOOM_CENTER_Y + sinf(angle) * layout::LOOM_RADIUS;
  spark.velocityX = (cosf(angle) - sinf(angle)) * speed;
  spark.velocityY = (sinf(angle) + cosf(angle)) * speed;
  spark.life = 30 + random(20);
}

/** Randomly decides whether to emit this frame; the chance grows with load above the 75 % threshold. */
static bool shouldEmit(float elapsedFrames) {
  float load = stats::busiestLoad();
  if (load <= EMISSION_THRESHOLD || activeCount >= MAX_SPARKS) return false;
  float chancePerThousand = (load - EMISSION_THRESHOLD) / 40 * 1000 * elapsedFrames;
  return random(1000) < (long)chancePerThousand;
}

void update(float elapsedFrames, bool emitting) {
  if (emitting && shouldEmit(elapsedFrames)) emitFromRim();
  for (int i = 0; i < activeCount;) {
    Spark& spark = pool[i];
    spark.x += spark.velocityX * elapsedFrames;
    spark.y += spark.velocityY * elapsedFrames;
    spark.velocityY += GRAVITY * elapsedFrames;
    spark.life -= elapsedFrames;
    const bool expired = spark.life <= 0 || !emitting;
    if (expired) pool[i] = pool[--activeCount];
    else i++;
  }
}

void draw() {
  M5Canvas& surface = canvas::surface();
  const bool critical = stats::busiestLoad() > 85;
  for (int i = 0; i < activeCount; i++) {
    const Spark& spark = pool[i];
    const bool outsideLoomArea = spark.y < layout::SPARK_TOP_LIMIT || spark.y > layout::SPARK_BOTTOM_LIMIT;
    if (outsideLoomArea) continue;
    uint16_t color = spark.life > 25 ? theme::amberPale : critical ? theme::alarmRed : theme::amberWarm;
    surface.fillRect((int)spark.x, canvas::toBand(spark.y), 2, 2, theme::blend(theme::background, color, spark.life / 20));
  }
}

}
