#pragma once

/**
 * @file navigation.h
 * @brief Button handling and screen selection.
 *
 * Left button (A) shows the previous screen, right button (C) the next one;
 * a click on the middle button (B) mutes or silences the alarm and holding it
 * toggles the LED bars. Screens wrap around.
 */
#include <stdint.h>

/** The available screens in navigation order. Count is the number of screens, not a screen. */
enum class Screen : uint8_t { Loom, History, Detail, Count };

namespace navigation {

/** Returns the screen currently shown. */
Screen current();
/** Returns the screen the right button would switch to. */
Screen next();
/** Returns the screen the left button would switch to. */
Screen previous();
/** Returns the upper-case display name of a screen, e.g. "LOOM". */
const char* nameOf(Screen screen);
/**
 * Processes button events for this loop and shows the button hints for
 * settings::BUTTON_HINT_MS after any press. Call after M5.update().
 * @param now Current time in ms (millis()).
 */
void handleButtons(uint32_t now);
/** Returns true while the button hint labels should be shown in the footer. */
bool hintsVisible(uint32_t now);

}
