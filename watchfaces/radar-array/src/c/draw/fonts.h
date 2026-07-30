/**
 * @file fonts.h
 * @brief Radar Array font slots, one per Share Tech Mono size the face loads.
 *
 * Named for the resource each one holds rather than the job it does, so a slot maps one to one
 * onto the manifest and two roles at the same size share a slot instead of loading it twice.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Listed in
 * registration order.
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
    FONT_STM_40,  // clock
    FONT_STM_18,  // date band, and the weather / hr / steps readouts
    FONT_STM_17,  // date band fallback for wide formats
    FONT_STM_14,  // lat/lon readout
    FONT_STM_12,  // overlay labels and the meridiem
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
