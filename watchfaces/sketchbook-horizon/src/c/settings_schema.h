/**
 * @file settings_schema.h
 * @brief Sketchbook settings schema, shared by every scene.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include "system/settings/settings.h"
#include "theme/palette_active.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief How the face arranges the clock, the date, and the readouts.
 *
 * Three steps along one line, each giving the clock more room by taking something else away.
 */
typedef enum
{
    LAYOUT_STANDARD  = 0, ///< Date under the clock, readouts along the bottom
    LAYOUT_DATE_TOP  = 1, ///< Date up on the strip, readouts kept, clock grows into the gap
    LAYOUT_BIG_CLOCK = 2, ///< Date up on the strip, readouts dropped, clock takes the lot
    LAYOUT_COUNT
} SketchbookLayout;

/**
 * @brief Gets the settings schema for the face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *sketchbook_settings_schema(void);

/**
 * @brief Which scene is drawn.
 *
 * The face's own setting rather than the library's, so it carries an id past SETTING_COUNT and is
 * read back here instead of through settings_u8.
 *
 * @return A SceneId value.
 */
uint8_t sketchbook_scene(void);

/**
 * @brief Which of the three arrangements is in force.
 *
 * @return A LAYOUT_* value.
 */
uint8_t sketchbook_layout(void);

/**
 * @brief Whether the AM/PM marker is drawn at all.
 *
 * Only says anything on a 12-hour clock, since the other formats have no marker.
 *
 * @return True to draw it.
 */
bool sketchbook_show_meridiem(void);

/**
 * @brief Fills a pick from the palette settings, ready for sketchbook_active_theme.
 *
 * It sets the mode, the day/night/Quiet themes, and whether each override is on. The night and
 * Quiet Time readings are live, so it leaves night_on and quiet_on off for the caller to fill from
 * the clock and Quiet Time.
 *
 * @param out The pick to fill.
 */
void sketchbook_palette_pick(PalettePick *out);

/** @} */
