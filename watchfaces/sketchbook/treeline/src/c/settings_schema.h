/**
 * @file settings_schema.h
 * @brief Treeline settings schema.
 *
 * @ingroup watchface-treeline
 */
#pragma once
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-treeline
 * @{
 */

/**
 * @brief The schema the settings library loads, saves and parses this face's blob with.
 *
 * @return The schema.
 */
const SettingsSchema *treeline_settings_schema(void);

/**
 * @brief How the face arranges the clock, the date, and the readouts.
 *
 * Three steps along one line, each giving the clock more room by taking something else away.
 */
typedef enum
{
    LAYOUT_STANDARD  = 0, ///< Clock over the clearing with the date under it and the readouts below
    LAYOUT_DATE_TOP  = 1, ///< Date up on the strip, a bigger clock, readouts kept
    LAYOUT_BIG_CLOCK = 2, ///< Readouts dropped and the clock fills the clearing
    LAYOUT_COUNT
} TreelineLayout;

/**
 * @brief Which of the three arrangements is in force.
 *
 * The face's own setting rather than the library's, so it carries an id past SETTING_COUNT and is
 * read back here instead of through settings_u8.
 *
 * @return A LAYOUT_* value.
 */
uint8_t treeline_layout(void);

/**
 * @brief Whether the AM/PM marker is drawn at all.
 *
 * Only says anything on a 12-hour clock, since the other formats have no marker. Another
 * face-only setting, so it is read back here too.
 *
 * @return True to draw it.
 */
bool treeline_show_meridiem(void);

/** @} */
