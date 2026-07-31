/**
 * @file settings_schema.h
 * @brief Shoreline settings schema.
 *
 * @ingroup watchface-shoreline
 */
#pragma once
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-shoreline
 * @{
 */

/**
 * @brief The schema the settings library loads, saves and parses this face's blob with.
 *
 * @return The schema.
 */
const SettingsSchema *shoreline_settings_schema(void);

/**
 * @brief How the face arranges the clock, the date, and the readouts.
 *
 * Three steps along one line, each giving the clock more room by taking something else away.
 */
typedef enum
{
    LAYOUT_STANDARD  = 0, ///< Clock over the sand with the date under it and the readouts below
    LAYOUT_DATE_TOP  = 1, ///< Date up on the strip, a bigger clock, readouts kept
    LAYOUT_BIG_CLOCK = 2, ///< Readouts dropped and the clock fills the beach
    LAYOUT_COUNT
} ShorelineLayout;

/**
 * @brief Which of the three arrangements is in force.
 *
 * The face's own setting rather than the library's, so it carries an id past SETTING_COUNT and is
 * read back here instead of through settings_u8.
 *
 * @return A LAYOUT_* value.
 */
uint8_t shoreline_layout(void);

/**
 * @brief Whether the AM/PM marker is drawn at all.
 *
 * Only says anything on a 12-hour clock, since the other formats have no marker. Another
 * face-only setting, so it is read back here too.
 *
 * @return True to draw it.
 */
bool shoreline_show_meridiem(void);

/** @} */
