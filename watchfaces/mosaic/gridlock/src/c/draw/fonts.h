/**
 * @file fonts.h
 * @brief Gridlock's font slots, one per font the face loads, named by the size you see on screen.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): main.c load_fonts registers a
 * handle under each, metrics.c carries the per-slot metrics, and every text drawer names the slot
 * from here. Concrete size names where the slot is a fixed custom font. Semantic names for the
 * header (swapped at runtime) and the slots that fall back to a system font.
 *
 * @ingroup gridlock_draw
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup gridlock_draw
 * @{
 */

enum
{
    FONT_TEKO_96,   // biggest night digits, fills a 2x2 with two glyphs
    FONT_TEKO_88,   // night digits, fills the wider "HH:MM" in a 2x4
    FONT_TEKO_72,   // big-digit night panels fallback (digits and colon)
    FONT_TEKO_54,   // 1x4 clock readout
    FONT_TEKO_46,   // clock readout
    FONT_TIME_SM,   // clock readout fallback for the wide .beats token (system font)
    FONT_STM_14,    // Share Tech Mono date banner
    FONT_DATE_SM,   // date banner fallback for wide formats (system font)
    FONT_DATE_XS,   // date banner fallback for the widest formats (system font)
    FONT_TEKO_26,   // standard 1x2 / stat value
    FONT_TEKO_22,   // tightest shrink fallback
    FONT_TEKO_24,   // one step below TEKO_26 (1x2 time shrink)
    FONT_TEKO_34,   // big stat values (heartrate / steps)
    FONT_COORD,     // lat / lon (system font)
    FONT_STM_12,    // Share Tech Mono caption / AM-PM unit
    FONT_HEADER,    // panel header label (user-selectable, swapped at runtime)
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
