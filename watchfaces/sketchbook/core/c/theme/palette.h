/**
 * @file palette.h
 * @brief The palette contract: the theme list every Sketchbook face offers, and the colours they all
 * paint with.
 *
 * A face's `Palette` is mostly its own — a ridge, a sea or a stand of firs needs colours nothing
 * else does. But the sky, the linework, the disc and the readouts are the same job on every one
 * of them, and the shared scene code paints with exactly those. So the family owns that block of
 * fields and the face pastes it at the top of its own struct.
 *
 * It is a macro rather than a nested struct on purpose: a face keeps writing `.sky_hi = ...`
 * flat, shared code keeps writing `pal->cloud`, and a face that drops a field fails to compile
 * rather than rendering it black.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup family-sketchbook
 * @{
 */

/**
 * @brief The nine colours every -line scene paints with.
 *
 * Pasted at the top of a face's own Palette, which then adds whatever its own landscape is made
 * of. Split by what a colour sits *on* rather than by what draws it: `ink` is linework over the
 * sky, `text` is the clock down on the ground, and the two are rarely the same once the ground
 * is dark.
 */
#define SKETCHBOOK_PALETTE_COMMON                                                          \
    GColor sky_hi;  /**< Sky at the top of the screen */                             \
    GColor sky_lo;  /**< Sky down where the scene meets it */                        \
    GColor ink;     /**< Linework over the sky: outlines and the guide arc */         \
    GColor text;    /**< The clock, over whatever the face's ground is */            \
    GColor dim;     /**< Date and the stats row */                                   \
    GColor bar_ink; /**< The status bar's contents, which sit on the dark strip */    \
    GColor disc;    /**< Sun or moon fill, and the sun's rays */                     \
    GColor cloud;   /**< Cloud fill, and the weather wash */                         \
    GColor precip;  /**< Rain, sleet and snow */

/**
 * @brief Themes the Clay picker offers, in its option order.
 *
 * Shared so the family is picked from one list of names rather than three. The count caps
 * KNOWN_THEME in each face's schema. What each theme actually looks like is the face's own
 * business: only the names and the order are agreed here.
 */
typedef enum
{
    THEME_SKETCHBOOK = 0,
    THEME_DAYBREAK   = 1,
    THEME_ALPENGLOW  = 2,
    THEME_BLUEPRINT  = 3,
    THEME_FOREST     = 4,
    THEME_MONO       = 5,
    THEME_CYBERPUNK  = 6,
    THEME_NEO_TOKYO  = 7,
    THEME_COUNT
} SketchbookTheme;

/** @} */
