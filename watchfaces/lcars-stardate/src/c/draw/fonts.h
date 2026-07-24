/**
 * @file fonts.h
 * @brief LCARS Stardate font slots, one per Antonio size the face loads, named by the size.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Every slot is
 * a fixed Antonio custom font, so all use concrete FONT_ANTONIO_<size> names.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-lcars
 * @{
 */

enum
{
    FONT_ANTONIO_62,  // clock
    FONT_ANTONIO_36,  // date banner
    FONT_ANTONIO_32,  // date banner fallback for wide formats
    FONT_ANTONIO_28,  // date banner fallback for the widest formats
    FONT_ANTONIO_20,  // weather temp
    FONT_ANTONIO_16,  // heartrate / steps / condition
    FONT_ANTONIO_12,  // lat / lon
    FONT_ANTONIO_10,  // AM/PM superscript
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
