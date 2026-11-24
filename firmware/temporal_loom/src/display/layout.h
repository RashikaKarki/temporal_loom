#pragma once

/**
 * @file layout.h
 * @brief Screen positions and sizes of every on-screen element.
 *
 * All values are in pixels in screen coordinates on the 320x240 display,
 * with (0, 0) at the top-left corner.
 */

namespace layout {

/** Height of the header bar. */
constexpr int HEADER_HEIGHT = 16;
/** Top of the header text. */
constexpr int HEADER_TEXT_Y = 4;
/** Horizontal centre of the clock in the header. */
constexpr int HEADER_CLOCK_X = 168;

/** Horizontal centre of the loom. */
constexpr int LOOM_CENTER_X = 90;
/** Vertical centre of the loom. */
constexpr int LOOM_CENTER_Y = 96;
/** Radius of the loom's outer rim; sparks are emitted from here. */
constexpr int LOOM_RADIUS = 70;
/** Radius of the divider circle between the CPU and GPU rings. */
constexpr int LOOM_DIVIDER_RADIUS = 40;
/** Inner radius of the CPU thread ring. */
constexpr int CPU_RING_INNER = 14;
/** Outer radius of the CPU thread ring. */
constexpr int CPU_RING_OUTER = 38;
/** Inner radius of the GPU thread ring. */
constexpr int GPU_RING_INNER = 43;
/** Outer radius of the GPU thread ring. */
constexpr int GPU_RING_OUTER = LOOM_RADIUS - 6;

/** Left edge of the meter column and its segment bars. */
constexpr int METER_LEFT = 194;
/** Right edge of the meter column; percentages are right-aligned here. */
constexpr int METER_RIGHT = 315;
/** Left edge of the temperature reading in a meter row. */
constexpr int METER_TEMPERATURE_X = 222;
/** Number of segments in each meter bar (10 % each). */
constexpr int METER_SEGMENTS = 10;
/** Horizontal distance between the starts of neighbouring meter segments. */
constexpr int METER_SEGMENT_PITCH = 12;
/** Top of the CPU meter row. */
constexpr int METER_ROW_CPU = 26;
/** Top of the GPU meter row. */
constexpr int METER_ROW_GPU = 62;
/** Top of the RAM meter row. */
constexpr int METER_ROW_RAM = 98;
/** Top of the VRAM meter row. */
constexpr int METER_ROW_VRAM = 134;

/** Baseline (0 %) of the recent-history strip on the loom screen. */
constexpr int STRIP_BASELINE_Y = 202;
/** Height of the recent-history strip at 100 %. */
constexpr int STRIP_HEIGHT = 26;
/** Top of the CPU/GPU labels above the history strip. */
constexpr int STRIP_LABEL_Y = 170;

/** Sparks above this row are not drawn. */
constexpr int SPARK_TOP_LIMIT = 18;
/** Sparks below this row are not drawn. */
constexpr int SPARK_BOTTOM_LIMIT = 200;

/** Top of the footer area used by the alarm banner. */
constexpr int FOOTER_TOP = 210;
/** Top of the footer text (top process, history legend). */
constexpr int FOOTER_TEXT_Y = 220;
/** Top of the button hint area. */
constexpr int HINT_TOP = 212;
/** Vertical centre of the button hint labels. */
constexpr int HINT_TEXT_Y = 228;
/** Horizontal centre of the left button hint. */
constexpr int HINT_LEFT_X = 65;
/** Horizontal centre of the middle button hint. */
constexpr int HINT_MIDDLE_X = 160;
/** Horizontal centre of the right button hint. */
constexpr int HINT_RIGHT_X = 255;

/** Left edge of the history graph; the percent axis labels sit just left of it. */
constexpr int GRAPH_LEFT = 30;
/** Top of the history graph (100 %). */
constexpr int GRAPH_TOP = 26;
/** Bottom of the history graph (0 %). */
constexpr int GRAPH_BOTTOM = 196;

/** Left edge of the labels on the detail screen. */
constexpr int DETAIL_LABEL_X = 8;
/** Left edge of the values on the detail screen. */
constexpr int DETAIL_VALUE_X = 92;
/** Top of the first row on the detail screen. */
constexpr int DETAIL_FIRST_ROW_Y = 26;
/** Vertical distance between rows on the detail screen. */
constexpr int DETAIL_ROW_SPACING = 24;

}
