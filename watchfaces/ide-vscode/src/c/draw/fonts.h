/**
 * @file fonts.h
 * @brief VS Code font slots, one per Teko / Share Tech Mono size the face loads, named by role.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Listed in
 * registration order, with the resource each slot loads.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

enum
{
    FONT_TIME,     // Teko 78 - clock
    FONT_TIME_SM,  // Teko 72 - clock fallback for wide strings
    FONT_DATE,     // STM 20 - date line
    FONT_VALUE,    // STM 14 - readout values (weather / steps / battery)
    FONT_DATE_SM,  // STM 12 - small date / tab-strip text
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
