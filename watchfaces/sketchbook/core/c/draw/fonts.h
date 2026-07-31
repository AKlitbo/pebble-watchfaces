/**
 * @file fonts.h
 * @brief The -line family's font slots, one per Patrick Hand size the face loads.
 *
 * Named for the resource each one holds rather than the job it does, so a slot maps one to one
 * onto the manifest and two roles at the same size share a slot instead of loading it twice.
 *
 * These are the ids passed to the lib font registry (ui/fonts.h): each face's layout.c
 * load_fonts registers a handle under each and its zone table names the slot from here. Listed
 * in registration order.
 *
 * Shared because every face in the family sets its clock in Patrick Hand at the same sizes. A
 * face that wanted its own type would keep its own slots instead.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include "ui/fonts.h"

/**
 * @addtogroup family-sketchbook
 * @{
 */

enum
{
    FONT_HAND_92,  // the roomier layouts' clock, sized so the widest time fits and never shrinks
    FONT_HAND_72,  // clock
    FONT_HAND_64,  // clock fallback for the wider .beats token
    FONT_HAND_22,  // date under the clock
    FONT_HAND_18,  // stats row, and the date fallback for the long formats
    FONT_HAND_16,  // stats fallback and the meridiem
    FONT_COUNT
};

_Static_assert(FONT_COUNT <= FONT_SLOTS_MAX, "more font slots than the registry holds; raise FONT_SLOTS_MAX");

/** @} */
