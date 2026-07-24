/**
 * @file fonts.h
 * @brief Radar Array font slots, one per Share Tech Mono size the face loads, named by role.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Listed in
 * registration order, with the resource each slot loads.
 *
 * @ingroup watchface-radar
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-radar
 * @{
 */

enum
{
    FONT_TIME,     // STM 40 - clock
    FONT_DATE,     // STM 18 - date band
    FONT_DATE_SM,  // STM 17 - date band fallback for wide formats
    FONT_SM,       // STM 18 - small labels
    FONT_VALUE,    // STM 18 - readout values (weather / hr / steps)
    FONT_COORD,    // STM 14 - lat/lon readout
    FONT_XS,       // STM 12 - overlay labels
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
