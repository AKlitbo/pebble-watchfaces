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

/** @} */
