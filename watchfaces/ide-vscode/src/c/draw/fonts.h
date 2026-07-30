/**
 * @file fonts.h
 * @brief VS Code font slots, one per Teko / Share Tech Mono size the face loads.
 *
 * Named for the resource each one holds rather than the job it does, so a slot maps one to one
 * onto the manifest and two roles at the same size share a slot instead of loading it twice.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): layout.c load_fonts
 * registers a handle under each and the zone table names the slot from here. Listed in
 * registration order.
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
    FONT_TEKO_78,  // clock
    FONT_TEKO_72,  // clock fallback for wide strings
    FONT_STM_20,   // date line
    FONT_STM_14,   // the weather / hr / steps readouts, and the battery
    FONT_STM_12,   // date fallback, and the condition fallback
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
