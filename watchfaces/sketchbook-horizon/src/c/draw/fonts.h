/**
 * @file fonts.h
 * @brief The face's font slots, one per Patrick Hand size it loads.
 *
 * Named for the resource each one holds rather than the job it does, so a slot maps one to one
 * onto the manifest and two roles at the same size share a slot instead of loading it twice.
 *
 * These are the ids passed to the framework's font registry (ui/fonts.h), and the shared slot
 * table in layout/zones.c names its fonts from here. load_fonts in layout/engine.c registers only
 * the sizes each watch draws, so the rectangle skips Hand 82 and the round screen skips Hand 92
 * and Hand 64. Listed in registration order.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

enum
{
    FONT_HAND_92,  // the roomier layouts' clock, sized so the widest time fits and never shrinks
    FONT_HAND_82,  // the round clock. 92 runs into the bezel down there, 72 reads small
    FONT_HAND_72,  // clock
    FONT_HAND_64,  // clock fallback for the wider .beats token
    FONT_HAND_22,  // date under the clock
    FONT_HAND_18,  // stats row, and the date fallback for the long formats
    FONT_HAND_16,  // stats fallback and the meridiem
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
