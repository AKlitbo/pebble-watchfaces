/**
 * @file icon_cache.h
 * @brief A small load-once icon store keyed by resource id, shared across a face so each
 * icon picture loads a single time and frees in one place. Alongside it sits the palette
 * tint and an optional auto-trim that measures each icon's transparent border so a caller
 * can pin the visible art instead of the padded bitmap box.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

// auto-trim measures each icon's transparent margin once on load so placement can follow
// the visible art. flip to 0 to fall back to pinning the full bitmap box
#define ICON_AUTOTRIM 1
// logs each icon's measured N/E/S/W on first load so you can cross-check the numbers.
// off by default in the shared lib so a normal build stays quiet. a face can flip it on
#define ICON_TRIM_LOG 0

/**
 * @brief The transparent border around an icon's opaque art, in pixels per side.
 */
typedef struct
{
    int8_t n;
    int8_t e;
    int8_t s;
    int8_t w;
} IconMargins;

/**
 * @brief Hands back an icon picture from the cache, loading it the first time it is
 * asked for.
 *
 * @param res The resource id.
 * @return The cached picture, or NULL if it could not be loaded.
 */
GBitmap *icon_get(uint32_t res);

/**
 * @brief The size of an icon in pixels. Gives back GSize(0,0) if it could not load.
 *
 * @param res The resource id.
 * @return The size of the picture.
 */
GSize icon_size(uint32_t res);

/**
 * @brief The measured transparent margins of a cached icon. Loads and measures it the
 * first time, then returns the cached numbers. All zero when auto-trim is off or the
 * icon could not load.
 *
 * @param res The resource id.
 * @return The N/E/S/W transparent margins.
 */
IconMargins icon_margins(uint32_t res);

/**
 * @brief Measures the transparent margins of any bitmap, for icons that live outside the
 * shared cache such as a weather picture that reloads as the weather changes. All zero
 * when auto-trim is off or the bitmap is NULL.
 *
 * @param bmp The bitmap to scan.
 * @return The N/E/S/W transparent margins.
 */
IconMargins icon_margins_of(GBitmap *bmp);

/**
 * @brief Paints an icon a different colour. It only works on icons saved as little
 * colour-table pictures. Other kinds it just leaves alone. The see-through pixels stay
 * see-through so the edges keep looking clean.
 *
 * @param bmp The picture to recolour (nothing happens if it is NULL).
 * @param color The colour to paint it.
 */
void icon_tint(GBitmap *bmp, GColor color);

/**
 * @brief The extra offset that pins the opaque box rather than the full bitmap at a given
 * anchor. A right or bottom edge kills the trailing margin. A left or top edge kills the
 * leading one. A centred axis splits the difference so the visible art centres.
 *
 * @param align The anchor the icon is pinned at.
 * @param margins The icon's transparent margins.
 * @param trim_dx Output for the sideways offset.
 * @param trim_dy Output for the up and down offset.
 */
void icon_align_trim(GAlign align, IconMargins margins, int *trim_dx, int *trim_dy);

/**
 * @brief Frees every icon picture in the cache.
 */
void icons_cleanup(void);

/** @} */
