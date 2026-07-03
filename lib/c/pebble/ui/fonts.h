/**
 * @file fonts.h
 * @brief Font registry: a face loads its custom fonts once under a category id, the rest
 * of the ui looks them up by id so layout tables can stay static (an id, not a
 * runtime GFont handle). single-platform binaries register their own handles, so
 * there is no per-platform font-set resolution here
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Identifier for each loaded font category.
 */
typedef enum
{
    FONT_TIME_BIG, // 1x4 clock readout
    FONT_TIME,    // clock readout
    FONT_TIME_SM, // clock readout fallback for the wide .beats token
    FONT_DATE,    // date banner
    FONT_DATE_SM, // date banner fallback for wide formats
    FONT_DATE_XS, // date banner fallback for the widest formats
    FONT_SM,      // weather temp
    FONT_SM_NARROW, // fallback for long strings in 1x2
    FONT_SM_MED,    // one step below FONT_SM (modular-emery: Teko 24). 1x2 time shrink
    FONT_VALUE,   // heartrate / steps values
    FONT_COORD,   // lat / lon
    FONT_XS,      // AM/PM superscript
    FONT_COUNT
} FontId;

/**
 * @brief Store a loaded font handle under its category id.
 *
 * @param id The category id.
 * @param handle The loaded font handle.
 */
void fonts_register(FontId id, GFont handle);

/**
 * @brief Resolve a category id to its handle. Falls back to the system font on a miss.
 *
 * @param id The category id.
 * @return The font handle.
 */
GFont fonts_get(FontId id);

/**
 * @brief Unload every registered font and clear the registry.
 */
void fonts_unload_all(void);

/** @} */
