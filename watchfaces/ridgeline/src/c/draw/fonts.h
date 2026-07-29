/**
 * @file fonts.h
 * @brief Ridgeline font slots, one per Patrick Hand size the face loads, named by role.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Listed in
 * registration order, with the resource each slot loads.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

enum
{
    FONT_TIME,     // Patrick Hand 72 - clock
    FONT_TIME_SM,  // Patrick Hand 64 - clock fallback for the wider .beats token
    FONT_DATE,     // Patrick Hand 22 - date under the clock
    FONT_DATE_SM,  // Patrick Hand 18 - date fallback for the long formats
    FONT_VALUE,    // Patrick Hand 18 - stats row
    FONT_XS,       // Patrick Hand 16 - stats fallback and the meridiem
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
