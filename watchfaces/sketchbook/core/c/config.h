/**
 * @file config.h
 * @brief The contract between a face and the family: the numbers the face must supply, and the
 * ones the family fixes for everyone.
 *
 * Shared drawing code needs to know where a face's sky, weather and ground sit, and those differ
 * on every face. They arrive as macros from the face's own sketchbook_config.h rather than as a struct
 * of values, for one reason: a struct field left out of a designated initializer compiles to
 * zero without a warning, and the sun would quietly draw at the top of the screen. A missing
 * macro cannot get that far. The #ifndef guards below name what is missing, and everywhere else
 * a macro is used in ordinary arithmetic, where a gap is an undeclared identifier.
 *
 * There is no cost to this: every face compiles its own copy of the family, so these end up as
 * the same immediates the constants they replaced already were.
 *
 * The one rule: never test a SKETCHBOOK_* config macro with #if. An undefined macro is 0 there and
 * would slip straight through. The guards below are the only place the preprocessor inspects
 * them.
 *
 * @ingroup family-sketchbook
 */
#pragma once

#include "sketchbook_config.h"  // the face's own, through its src/c include root

/**
 * @addtogroup family-sketchbook
 * @{
 */

// --- what the family fixes ---
// these do not vary, so they are not the face's to decide
#define SKETCHBOOK_ARC_CX 100  ///< The arc is centred on the screen for every face in the family
#define SKETCHBOOK_DISC_R  12  ///< And every face's sun and moon are the same size

// --- what the face owes us ---

#ifndef SKETCHBOOK_CLOUD_DROP
#error "sketchbook_config.h must define SKETCHBOOK_CLOUD_DROP: how far under the disc a drifting cloud parks"
#endif
#ifndef SKETCHBOOK_CLOUD_Y_MIN
#error "sketchbook_config.h must define SKETCHBOOK_CLOUD_Y_MIN: the highest a drifting cloud may park"
#endif
#ifndef SKETCHBOOK_CLOUD_Y_MAX
#error "sketchbook_config.h must define SKETCHBOOK_CLOUD_Y_MAX: the lowest, keeping it off the ground"
#endif
#ifndef SKETCHBOOK_DRIFT_LOBE_A
#error "sketchbook_config.h must define SKETCHBOOK_DRIFT_LOBE_A: the lobe radius of the drifting cloud's near puff"
#endif
#ifndef SKETCHBOOK_DRIFT_LOBE_B
#error "sketchbook_config.h must define SKETCHBOOK_DRIFT_LOBE_B: and of its far one"
#endif
#ifndef SKETCHBOOK_OVERCAST_PUFFS
#error "sketchbook_config.h must define SKETCHBOOK_OVERCAST_PUFFS: the overcast band, as a Puff initialiser"
#endif
#ifndef SKETCHBOOK_PRECIP_TOP
#error "sketchbook_config.h must define SKETCHBOOK_PRECIP_TOP: the top of the band the fall is spread over"
#endif
#ifndef SKETCHBOOK_PRECIP_H
#error "sketchbook_config.h must define SKETCHBOOK_PRECIP_H: how deep that band runs"
#endif
#ifndef SKETCHBOOK_WASH_TOP
#error "sketchbook_config.h must define SKETCHBOOK_WASH_TOP: where the weather wash starts thinning out"
#endif
#ifndef SKETCHBOOK_WASH_BOTTOM
#error "sketchbook_config.h must define SKETCHBOOK_WASH_BOTTOM: where it stops"
#endif
#ifndef SKETCHBOOK_WASH_WISP_HI_Y
#error "sketchbook_config.h must define SKETCHBOOK_WASH_WISP_HI_Y: the upper solid wisp through the wash"
#endif
#ifndef SKETCHBOOK_WASH_WISP_LO_Y
#error "sketchbook_config.h must define SKETCHBOOK_WASH_WISP_LO_Y: and the lower one"
#endif
#ifndef SKETCHBOOK_BOLT_STROKE
#error "sketchbook_config.h must define SKETCHBOOK_BOLT_STROKE: the fork, as a GPoint[][2] initialiser"
#endif

// a number that is present but impossible is caught here rather than on the watch
_Static_assert(SKETCHBOOK_CLOUD_Y_MIN <= SKETCHBOOK_CLOUD_Y_MAX, "the drifting cloud's clamp is inverted");
_Static_assert(SKETCHBOOK_WASH_TOP < SKETCHBOOK_WASH_BOTTOM, "the wash runs down the screen, not up it");
_Static_assert(SKETCHBOOK_WASH_TOP <= SKETCHBOOK_WASH_WISP_HI_Y && SKETCHBOOK_WASH_WISP_HI_Y <= SKETCHBOOK_WASH_BOTTOM,
    "the upper wisp has to sit inside the wash it is meant to give an edge to");
_Static_assert(SKETCHBOOK_WASH_TOP <= SKETCHBOOK_WASH_WISP_LO_Y && SKETCHBOOK_WASH_WISP_LO_Y <= SKETCHBOOK_WASH_BOTTOM,
    "and so does the lower one");
_Static_assert(SKETCHBOOK_PRECIP_TOP + SKETCHBOOK_PRECIP_H <= 228, "the fall has to stay on the screen");

/** @} */
