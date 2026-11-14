/**
 * @file widgets.h
 * @brief Overlay painters, exposed as stateless draws the engine's overlays draw-slot calls: the
 * battery, the weather condition glyph (swapped per condition and tinted), and the Quiet Time and
 * bluetooth status. Each one reads where it goes and in what colour from the desktop's layout.
 * The glyphs come from the shared icon cache and the engine owns the layer.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "desktop/desktop.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

/**
 * @brief Draw the battery, bluetooth, and Quiet Time status.
 *
 * Battery Display hides the battery icon or the percentage, and a desktop whose percentage has a
 * labelled spot of its own shows it whatever the setting says. Show Connection Icon hides
 * bluetooth. On a desktop with glyphs, the Quiet Time glyph only shows while Quiet Time is on,
 * and everything packs leftwards from the battery icon with anything hidden closing up. On a
 * desktop with words, each sits on its own line as LINK or DOWN and ON or OFF.
 *
 * Nothing wakes the face when Quiet Time starts or ends, so its status follows on the next
 * minute's repaint.
 *
 * @param ctx The graphics context.
 * @param layout The active desktop layout.
 * @param level Battery charge level percentage.
 * @param connected True when the phone link is up.
 */
void widgets_draw_status(GContext *ctx, const DesktopLayout *layout, int level, bool connected);

/**
 * @brief Draw the clock a glyph at a time, on a desktop whose clock_pitch is set, and nothing
 * otherwise.
 *
 * Each digit takes clock_pitch and the colon clock_colon_pitch, narrower than the font's own
 * advance, so a clock at the one size its font renders cleanly fits a space too narrow for it at
 * normal spacing. The whole time sits centred in the clock zone's rect.
 *
 * @param ctx The graphics context.
 * @param layout The active desktop layout.
 */
void widgets_draw_spaced_clock(GContext *ctx, const DesktopLayout *layout);

/**
 * @brief Draw the weather condition glyph in the desktop's tint, on a desktop that shows one.
 *
 * The glyph comes from the shared icon cache, so each condition's picture loads once.
 *
 * @param ctx The graphics context.
 * @param layout The active desktop layout.
 * @param condition The weather condition token.
 */
void widgets_draw_weather(GContext *ctx, const DesktopLayout *layout, const char *condition);

/** @} */
