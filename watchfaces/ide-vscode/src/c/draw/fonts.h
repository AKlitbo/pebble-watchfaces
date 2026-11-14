/**
 * @file fonts.h
 * @brief The font slots, one per job a font does on screen.
 *
 * Each desktop draws in its own fonts, so a slot names the job and the desktop's layout table
 * names the resource that fills it (see desktop/desktop.c). Two jobs a desktop sets in the same
 * resource share one loaded font rather than loading it twice.
 *
 * These are the ids passed to the framework's font registry (ui/fonts.h), and the zone tables
 * name the slot from here.
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
    FONT_CLOCK,        // clock
    FONT_CLOCK_SMALL,  // clock fallback for the wider .beats reading
    FONT_DATE,         // date line
    FONT_TEXT,         // the readouts and the battery
    FONT_SMALL,        // fallback for a date or a readout too wide for its own font
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
