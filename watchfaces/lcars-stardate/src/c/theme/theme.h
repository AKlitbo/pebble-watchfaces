/**
 * @file theme.h
 * @brief Theme to background-resource and accent-colour mapping.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief Maps the Theme setting (0=Classic, 1=Lower Decks, 2=PADD, 3=Nemesis Blue,
 * 4=Classic Mono, 5=Lower Decks Mono, 6=PADD Mono, 7=Voyager, 8=Voyager Mono) to
 * its baked LCARS background resource.
 *
 * @param theme The theme setting value.
 * @return The background resource ID.
 */
uint32_t bg_resource_for_theme(uint8_t theme);

/**
 * @brief Maps the Theme setting to its LCARS label text colour.
 *
 * @param theme The theme setting value.
 * @return The label colour.
 */
GColor label_color_for_theme(uint8_t theme);

/**
 * @brief The theme's top-left cap colour, so painted overlays (e.g. the battery gauge)
 * can match the panel they sit on instead of staying a fixed classic violet.
 *
 * @param theme The theme setting value.
 * @return The accent colour.
 */
GColor panel_accent_for_theme(uint8_t theme);

/**
 * @brief The rounded end-cap colour of the LCARS header bars.
 *
 * The bars are drawn rather than baked, so their two colours come from here instead of from the
 * frame artwork.
 *
 * @param theme The theme setting value.
 * @return The cap colour.
 */
GColor bar_cap_for_theme(uint8_t theme);

/**
 * @brief The long middle colour of the LCARS header bars.
 *
 * @param theme The theme setting value.
 * @return The bar body colour.
 */
GColor bar_body_for_theme(uint8_t theme);

/**
 * @brief The lit-segment colour for the battery gauge at a given charge level.
 *
 * Color themes warn with red (critical) and amber (low). Mono themes stay grayscale,
 * leaving the lit-segment count to signal charge.
 *
 * @param theme The theme setting value.
 * @param level Battery charge level percentage.
 * @return The fill colour for lit segments.
 */
GColor battery_fill_for_theme(uint8_t theme, int level);

/** @} */
