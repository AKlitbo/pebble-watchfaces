/**
 * @file settings_schema.h
 * @brief lcars-stardate-emery settings schema: version 2, with a v0 migration that lifts
 * its pre-versioning blob. key 5 and the v1 size are frozen. Changing either makes
 * watches in the field lose their settings.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *lcars_settings_schema(void);

/**
 * @brief What the upper left slot is set to show.
 *
 * The face's own settings rather than the library's, so they carry ids past SETTING_COUNT and are
 * read back here instead of through settings_u8.
 *
 * @return An OpsId.
 */
uint8_t lcars_slot_lt(void);

/**
 * @brief What the lower left slot is set to show. Ignored while the upper left holds the tall
 * weather block, which fills the whole column.
 *
 * @return An OpsId.
 */
uint8_t lcars_slot_lb(void);

/**
 * @brief What the upper right slot is set to show.
 *
 * @return An OpsId.
 */
uint8_t lcars_slot_rt(void);

/**
 * @brief What the lower right slot is set to show.
 *
 * @return An OpsId.
 */
uint8_t lcars_slot_rb(void);

/**
 * @brief How far the first alternate time zone runs from UTC.
 *
 * The setting arrives from Clay's location search as "offset,City, Region, CC", so this is the
 * number in front of the first comma.
 *
 * @return Minutes ahead of UTC, negative behind it. Zero when nothing is set.
 */
int16_t lcars_zone_1_offset_minutes(void);

/**
 * @brief The first alternate time zone's place name, for the holder box.
 *
 * @return Everything after the first comma, or "" when nothing is set. Never NULL.
 */
const char *lcars_zone_1_name(void);

/** @} */
