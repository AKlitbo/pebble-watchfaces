/**
 * @file fonts.h
 * @brief sidereel's font slots, one per font the face loads, named by the size you see on screen.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c registers a handle
 * under each and every drawer names its slot from here.
 *
 * The list is in two halves. The first is the face's own: the reel, the pennant, and the day
 * track. The second is the set gridlock's panels name, slot for slot, so a panel body copied from
 * that face needs no edit. Not every gridlock slot gets a handle registered, since the big-clock
 * sizes only ever come up in cells bigger than the one this face has. An unregistered slot falls
 * back to a system font rather than crashing, which is the right behaviour for a size that should
 * never be reached.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

enum
{
    // --- sidereel's own ---
    FONT_CLOCK_56,  // the minute rows and the hour in the pointer, one size for both

    // --- the gridlock panel set ---
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
