/**
 * @file settings_schema.h
 * @brief Ridgeline settings schema.
 *
 * A brand-new face, so it starts clean at version 1 with no migration history.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *ridgeline_settings_schema(void);

/**
 * @brief How the face arranges the clock, the date, and the readouts.
 *
 * Three steps along one line: each one gives the clock more room by taking something else away,
 * so a user picks how much of the rest of it they actually want.
 */
typedef enum
{
    LAYOUT_STANDARD  = 0, ///< Date under the clock, readouts along the bottom
    LAYOUT_DATE_TOP  = 1, ///< Date up on the strip, readouts kept, clock grows into the gap
    LAYOUT_BIG_CLOCK = 2, ///< Date up on the strip, readouts dropped, clock takes the lot
    LAYOUT_COUNT
} RidgelineLayout;

/**
 * @brief Which of the three arrangements is in force.
 *
 * This is the face's own setting rather than one of the library's, so it carries an id past
 * SETTING_COUNT and is read back here instead of through settings_u8.
 *
 * @return A LAYOUT_* value.
 */
uint8_t ridgeline_layout(void);

/**
 * @brief Whether the AM/PM marker is drawn at all.
 *
 * On a 24-hour or .beats clock there is no marker to begin with, so this only has anything to
 * say on a 12-hour one. Another of the face's own settings, so it is read back here rather than
 * through settings_u8.
 *
 * @return True to draw it.
 */
bool ridgeline_show_meridiem(void);

/** @} */
