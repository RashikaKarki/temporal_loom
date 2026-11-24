#pragma once

/**
 * @file theme.h
 * @brief Amber CRT colour palette and colour helpers.
 *
 * All colours are RGB565 and are only valid after theme::init() has run.
 */
#include <stdint.h>

namespace theme {

/** Screen background, a very dark brown. */
extern uint16_t background;
/** Colour of the faint horizontal scanlines drawn over the background. */
extern uint16_t scanline;
/** Primary amber used for normal text, values and the CPU series. */
extern uint16_t amber;
/** Dim amber for labels, frames and inactive elements. */
extern uint16_t amberDim;
/** Brighter amber for values above settings::WARM_LEVEL. */
extern uint16_t amberWarm;
/** Pale amber for highlights, peak markers and the GPU series. */
extern uint16_t amberPale;
/** Darker amber for the RAM series on the history graph. */
extern uint16_t ramAmber;
/** Red for critical values and the alarm. */
extern uint16_t alarmRed;
/** Colour of unlit meter segments and graph grid lines. */
extern uint16_t emptySegment;
/** Grey used for the header and values while the PC link is down. */
extern uint16_t offlineGrey;
/** Darker grey for the broken loom and graph lines while the PC link is down. */
extern uint16_t offlineGreyDark;

/** Initialises the palette colours. Call once in setup() before any drawing. */
void init();
/**
 * Linearly blends two RGB565 colours.
 * @param amount Blend factor, clamped to 0 (from) .. 1 (to).
 */
uint16_t blend(uint16_t from, uint16_t to, float amount);
/**
 * Returns the colour for a reading: amber normally, amberWarm above
 * settings::WARM_LEVEL and alarmRed above settings::CRITICAL_LEVEL.
 * @param level Reading in percent or °C.
 */
uint16_t heatColor(float level);

}
