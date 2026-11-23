/**
 * @file zones.h
 * @brief The clock, date, and readout slots every scene shares, and where they move for each
 * layout mode.
 *
 * Every scene wears the same clock, date, and stats row in the same places, so the slot table
 * lives here once. The one thing that moves from scene to scene is how low the Clock Only clock
 * sits on the rectangle, which each scene hands over as a single row.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "ui/zone.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief The shared slot table, ZONE_COUNT entries indexed by the engine's zone ids.
 *
 * Only one scene is up at a time, so they all share it. layout_zones_apply sets every field a
 * layout mode moves, so a table left by another scene or another mode carries nothing over.
 *
 * @return The slot table.
 */
Zone *layout_zones(void);

// AM/PM sits beside the digits on the rectangle's standard layout, AM left and PM right, so the
// half of the day shows in its position as well as its letters
#define LAYOUT_MERIDIEM_BESIDE_TOP 131  ///< 2px under the digits' cap line

/**
 * @brief Point the time, date, and readout slots at whichever layout mode is in force, and note
 * where the AM/PM marker goes and whether the stats row has a heart rate.
 *
 * Called before every rebuild, so a settings change or a heart rate coming and going lands
 * without a second table. A round screen only ships the standard layout, which the table already
 * holds, so there it only notes the marker.
 *
 * @param clock_only_y The top of the Clock Only clock's box on the rectangle, the one row each
 * scene picks for itself.
 */
void layout_zones_apply(int16_t clock_only_y);

/**
 * @brief The cap line an AM/PM marker above the colon sits on, for the layout the last
 * layout_zones_apply set up.
 *
 * Only the two Hand 92 layouts put it there on the rectangle, and their digits sit at different
 * heights. The round standard layout draws Hand 82 at its own height, so it gets its own channel.
 *
 * @return The marker's cap line.
 */
int16_t layout_meridiem_above_top(void);

#if !defined(PBL_ROUND)
/**
 * @brief Whether the stats row was laid out with a heart rate, as of the last layout_zones_apply.
 *
 * The slots, the readouts, and the glyphs all go by this one answer, so they agree with each other
 * until the next rebuild picks up a sensor that came or went.
 *
 * @return True when the row is three across.
 */
bool layout_zones_has_hr(void);
#endif

/** @} */
