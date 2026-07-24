/**
 * @file theme.h
 * @brief Theme to background-resource and accent-colour mapping.
 *
 * @ingroup watchface-radar
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-radar
 * @{
 */

/**
 * @brief Maps the Theme setting (0=Default, 1=Rescue, 2=Neon, 3=Stealth,
 * 4=Phosphor, 5=Crimson, 6=Mono) to its baked frame background resource.
 *
 * @param theme The theme setting value.
 * @return The background resource ID.
 */
uint32_t bg_resource_for_theme(uint8_t theme);

/**
 * @brief The theme's chrome accent, so painted overlays (the battery gauge) match the
 * frame chrome instead of staying a fixed amber.
 *
 * @param theme The theme setting value.
 * @return The accent colour.
 */
GColor panel_accent_for_theme(uint8_t theme);

/**
 * @brief The theme's primary phosphor colour, for the time and live readouts.
 *
 * @param theme The theme setting value.
 * @return The primary colour.
 */
GColor primary_color_for_theme(uint8_t theme);

/**
 * @brief The lit-segment colour for the battery gauge at a given charge level.
 *
 * Color themes warn with red (critical) and amber (low). The Mono theme stays
 * grayscale, leaving the lit-segment count to signal charge.
 *
 * @param theme The theme setting value.
 * @param level Battery charge level percentage.
 * @return The fill colour for lit segments.
 */
GColor battery_fill_for_theme(uint8_t theme, int level);

/** @} */
