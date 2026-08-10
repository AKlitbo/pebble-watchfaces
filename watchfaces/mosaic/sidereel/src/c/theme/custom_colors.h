/**
 * @file custom_colors.h
 * @brief The per-panel colours and header/border flags the appearance page writes.
 *
 * Both travel in one packed string on the APPEARANCE_CUSTOM_COLORS key, which can outgrow a
 * single 256 byte persist slot, so it is split into two blobs on the way in and rejoined on the
 * way out. That split is why this owns its own schema links rather than living in the main
 * settings struct.
 *
 * The colours only apply under THEME_CUSTOM. The header and border flags apply under every theme.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "mosaic/engine/catalog.h"
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief One panel's custom colours, plus a flag per channel saying whether it was set.
 *
 * A channel the user never picked keeps its flag clear so the panel leaves it mono.
 */
typedef struct
{
    uint8_t accent_set : 1, value_set : 1, icon_set : 1, subtitle_set : 1;
    GColor  accent, value, icon, subtitle;
} SidereelCustomColor;

/**
 * @brief The custom colours chosen for a panel.
 *
 * @param module The module type.
 * @return Its colours and which channels were set.
 */
SidereelCustomColor sidereel_custom_color(uint8_t module);

/**
 * @brief Whether a panel should drop its header strip at a given size.
 *
 * @param module The module type.
 * @param size The placement size.
 * @return True to draw it without a header.
 */
bool sidereel_module_headerless(uint8_t module, ModuleSize size);

/**
 * @brief Whether a panel should drop its outer border at a given size.
 *
 * @param module The module type.
 * @param size The placement size.
 * @return True to draw it without a border.
 */
bool sidereel_module_borderless(uint8_t module, ModuleSize size);

/**
 * @brief Store a freshly arrived combined string, splitting it across the two blobs.
 *
 * Wired to appmessage_on_custom_colors.
 *
 * @param combined The "~3 colours | flags" string.
 */
void sidereel_set_custom_colors(const char *combined);

/**
 * @brief Rebuild the combined string from the two blobs.
 *
 * Wired to appmessage_set_custom_colors_provider so the config page reopens on what the watch
 * actually holds.
 *
 * @param out The buffer to write into.
 * @param n Its size.
 */
void sidereel_get_custom_colors(char *out, size_t n);

/** @brief The head of this unit's schema links, for the main schema to chain onto. */
extern const SettingsSchema sidereel_custom_theme_schema;

/** @} */
