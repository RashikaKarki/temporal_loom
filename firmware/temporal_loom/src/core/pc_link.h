#pragma once

/**
 * @file pc_link.h
 * @brief Serial link to the PC companion.
 *
 * Reads newline-terminated JSON stats lines from USB serial, hands each one to
 * the stats store and tracks whether the PC is still sending.
 */
#include <stdint.h>

namespace pc_link {

/** Opens the serial port at settings::SERIAL_BAUD with an enlarged 1024-byte receive buffer. */
void begin();
/**
 * Consumes all pending serial bytes and applies every complete line.
 * Carriage returns are ignored; lines longer than 383 characters are discarded.
 * @param now Current time in ms, recorded when a valid line is applied.
 */
void poll(uint32_t now);
/**
 * Returns true when a valid stats line has been received within the last
 * settings::LINK_TIMEOUT_MS milliseconds.
 * @param now Current time in ms (millis()).
 */
bool isConnected(uint32_t now);

}
